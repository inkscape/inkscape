#!/usr/bin/env ruby

# simplepath.rb
# functions for digesting paths into a simple list structure
#
# Ruby port by MenTaLguY
#
# Copyright (C) 2005 Aaron Spike  <aaron@ekips.org>
# Copyright (C) 2006 MenTaLguY  <mental@rydia.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

require 'strscan'

def lexPath(d)
    # iterator which breaks path data 
    # identifies command and parameter tokens

    scanner = StringScanner.new(d)

    delim = /[ \t\r\n,]+/
    command = /[MLHVCSQTAZmlhvcsqtaz]/
    parameter = /(([-+]?[0-9]+(\.[0-9]*)?|[-+]?\.[0-9]+)([eE][-+]?[0-9]+)?)/

    until scanner.eos?
        scanner.skip(delim)
        if m = scanner.scan(command)
            yield m, true
        elsif m = scanner.scan(parameter)
            yield m, false
        else
            #TODO: create new exception
            raise 'Invalid path data!'
        end
    end
end

PathDef = Struct.new :implicit_next, :param_count, :casts, :coord_types
PATHDEFS = {
    'M' => PathDef['L', 2, [:to_f, :to_f], [:x,:y]], 
    'L' => PathDef['L', 2, [:to_f, :to_f], [:x,:y]], 
    'H' => PathDef['H', 1, [:to_f], [:x]], 
    'V' => PathDef['V', 1, [:to_f], [:y]], 
    'C' => PathDef['C', 6, [:to_f, :to_f, :to_f, :to_f, :to_f, :to_f], [:x,:y,:x,:y,:x,:y]], 
    'S' => PathDef['S', 4, [:to_f, :to_f, :to_f, :to_f], [:x,:y,:x,:y]], 
    'Q' => PathDef['Q', 4, [:to_f, :to_f, :to_f, :to_f], [:x,:y,:x,:y]], 
    'T' => PathDef['T', 2, [:to_f, :to_f], [:x,:y]], 
    'A' => PathDef['A', 7, [:to_f, :to_f, :to_f, :to_i, :to_i, :to_f, :to_f], [0,0,0,0,0,:x,:y]], 
    'Z' => PathDef['L', 0, [], []]
}

def parsePath(d)
    # Parse SVG path and return an array of segments.
    # Removes all shorthand notation.
    # Converts coordinates to absolute.

    retval = []

    command = nil
    outputCommand = nil
    params = []

    pen = [0.0,0.0]
    subPathStart = pen
    lastControl = pen
    lastCommand = nil

    lexPath(d) do |token, isCommand|
        raise 'Invalid number of parameters' if command and isCommand

        unless command
            if isCommand
                raise 'Invalid path, must begin with moveto.' \
                  unless lastCommand or token.upcase == 'M'
                command = token
            else
                #command was omited
                #use last command's implicit next command
                raise 'Invalid path, no initial command.' unless lastCommand
                if lastCommand =~ /[A-Z]/
                    command = PATHDEFS[lastCommand].implicit_next
                else
                    command = PATHDEFS[lastCommand.upcase].implicit_next.downcase
                end
            end
            outputCommand = command.upcase
        end

        unless isCommand
            param = token.send PATHDEFS[outputCommand].casts[params.length]
            if command =~ /[a-z]/
                case PATHDEFS[outputCommand].coord_types[params.length]
                when :x: param += pen[0]
                when :y: param += pen[1]
                end
            end
            params.push param
        end

        if params.length == PATHDEFS[outputCommand].param_count

            #Flesh out shortcut notation
            case outputCommand
            when 'H','V'
                case outputCommand
                when 'H': params.push pen[1]
                when 'V': params.unshift pen[0]
                end
                outputCommand = 'L'
            when 'S','T'
                params.unshift(pen[1]+(pen[1]-lastControl[1]))
                params.unshift(pen[0]+(pen[0]-lastControl[0]))
                case outputCommand
                when 'S': outputCommand = 'C'
                when 'T': outputCommand = 'Q'
                end
            end

            #current values become "last" values
            case outputCommand
            when 'M'
                subPathStart = params[0,2]
                pen = subPathStart
            when 'Z'
                pen = subPathStart
            else
                pen = params[-2,2]
            end

            case outputCommand
            when 'Q','C'
                lastControl = params[-4,2]
            else
                lastControl = pen
            end

            lastCommand = command
            retval.push [outputCommand,params]
            command = nil
            params = []
        end
    end

    raise 'Unexpected end of path' if command

    return retval
end

def formatPath(a)
    # Format SVG path data from an array
    a.map { |cmd,params| "#{cmd} #{params.join(' ')}" }.join
end

def _transformPath(p)
    p.each do |cmd,params|
        coord_types = PATHDEFS[cmd].coord_types
        for i in 0...(params.length)
            yield params, i, coord_types[i]
        end
    end
end

def translatePath(p, x, y)
    _transformPath(p) do |params, i, coord_type|
        case coord_type
        when :x: params[i] += x
        when :y: params[i] += y
        end
    end
end

def scalePath(p, x, y)
    _transformPath(p) do |params, i, coord_type|
        case coord_type
        when :x: params[i] *= x
        when :y: params[i] *= y
        end
    end
end

def rotatePath(p, a, cx = 0, cy = 0)
    return p if a == 0
    _transformPath(p) do |params, i, coord_type|
        if coord_type == :x
            x = params[i] - cx
            y = params[i + 1] - cy
            r = Math.sqrt((x**2) + (y**2))
            unless r.zero?
                theta = Math.atan2(y, x) + a
                params[i] = (r * Math.cos(theta)) + cx
                params[i + 1] = (r * Math.sin(theta)) + cy
            end
        end
    end
end
