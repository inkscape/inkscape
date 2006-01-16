#!/usr/bin/perl -w
#
# SpSVG
# 
# Perl module for sodipodi extensions
#
# This is a temporary hack that provides the following:
#   * Some standard getopts (help, i/o, ids)
#   * A way to exit that produces the error codes outlined in
#     the extension specs (SpSVG::error)
#   * A method that takes a function as its arguments and passes
#     each specified element ('--id=foo --id=bar', 'ids=fooz,baaz',
#     and so forth) as plain text to the function. The function is 
#     expected to return the processed version of this text.
#     
# TODO:
#
#   * Write POD
#   * Exit with a friendly message if XML::XQL isn't installed
#   * Decide how to implement the module interface
#   * Move from XML::XQL to SVG/SVG::Parser (see below)
#   * Make the process method more efficient (again, see below)
#
# Authors: Daniel Goude (goude@dtek.chalmers.se)
#

package SpSVG; # Think of a better name
use strict;
#use Carp;
use Exporter;
use Getopt::Long;
#use Data::Dumper; # For debugging

# From the SVG.pm documentation (actually 
# http://roasp.com/tutorial/tutorial6.shtml):
#
# > Currently, version 2.0 of SVG.pm does not internally support DOM
# > traversiong functionality such as getting the children,siblings,or
# > parent of an element, so the interaction capability between SVG::Parser
# > and SVG is limited to manipulations of a known image. The next version
# > of SVG will support all these and more key functions which will make
# > SVG::Parser extremely useful.
#
# I plan to replace the /XML::XQL(::DOM)?/ code as soon as this is
# fixed.

#use SVG;
#use SVG::Parser;

use XML::XQL;
use XML::XQL::DOM;

use vars qw(@ISA @EXPORT $VERSION);

$VERSION = 1.02; # fixme: use SpSVG 1.01 doesn't raise exception.
@ISA = qw(Exporter);

# Symbols 
@EXPORT = qw(

); 

sub new {
    my $self = {
        status   => make_status(),
        name     => '',      # Name of script
        usage    => '',      # Usage string
        opt_help => [],      # Used for --help
        
        ids     => [],       # Array of ids that will be iterated over 
                             # in process()
        svg     => '',       # SVG document object
        
    };
    bless $self;
}

sub parse {
    my $self = shift;
    
    my $infile = $self->{'opts'}->{'file'};

    my $xml;
    {
        local $/=undef;
        if ($infile) {
            open (IN, $infile) or 
                $self->error('IO_ERR', "Can't open $infile: $!\n");
            $xml = <IN>;
            close IN or 
                $self->error('IO_ERR', "Can't close $infile: $!\n");
        } else {
            $xml = <>;
        }
    }


    $self->{'parser'} = new XML::DOM::Parser;
    my $parser = $self->{'parser'};
    my $svg = $parser->parse($xml) ||
            $self->error('INPUT_ERR', "Couldn't parse input: $!.");
    $self->{'svg'} = $svg;
}

# Return SVG document as a string
sub get {
    my $self = shift;
    my $string =  $self->{'svg'}->toString;
    
}

# Print to $outfile|STDOUT
sub dump {
    my $self = shift;
    my $outfile = $self->{'opts'}->{'output'};
    if ($outfile) {
        open(OUT, ">$outfile") or 
            $self->error('IO_ERR', "Can't open $outfile for writing: $!\n");
        print OUT $self->get;
        close OUT or $self->error('IO_ERR', "Can't close $outfile: $!\n");
    } else {
        print $self->get;
    }
}

sub process_ids {
    my $self = shift;
    my $func = shift;

    my @ids = @{$self->{'ids'}};

    # Apply a user supplied function to each id
    foreach my $id (@ids) {
        my $svg = $self->{'svg'};
        #warn "ID: $id\n";
        my @nodes = $svg->xql("//*[\@id = '$id']") or
            $self->error('NOOP_ERR', "Couldn't find element $id.");
        my $node = shift @nodes; # Ids are unique
                                 # fixme: Add more checking.

        # Call the user function on the node identified by $id
        my $new_node = $func->($node->toString);
    
        # Replace the comment with user generated SVG
        my $parent = $node->getParentNode;
        my $comment = $svg->createComment('SpSVG');
        $parent->replaceChild($comment, $node);
        my $output =  $self->{'svg'}->toString;
        $output =~ s/<!--SpSVG-->/$new_node/;

        # Here the whole (new) document is parsed. Probably VERY inefficient,
        # but at least you get syntax checking for free..
        $self->{'svg'} = $self->{'parser'}->parse($output);
        #print $self->{'svg'}->toString;
    }

    
} 

# Exit status codes
sub make_status {
    my $self = shift;
    my %status = (
        0 => ["SUCCESS", "Extension exited gracefully"],
        1 => ["GEN_FAIL", "General failure"],
        2 => ["MEM_ERR", "Memory error"],
        3 => ["IO_ERR", "File I/O error"],
        4 => ["MATH_ERR", "Math error"],
        5 => ["INPUT_ERR", "Input not understood (not valid SVG)"],
        6 => ["NOOP_ERR", "Could not operate on any objects in this " . 
            "data stream"],
        7 => ["ARG_ERR", "Incorrect script arguments"]
    );

    # Generate error subs dynamically
    foreach my $exit_code (sort keys %status) {
        eval "sub $status{$exit_code}[0] { $exit_code; }";
        die $@ if $@;
    }
    return \%status;

}

# Create an option array suitable for Getopt::Long
sub make_opt_vals {
    my $self = shift;
    my @opt_desc = @_;
    my @opt_vals;
    my @opt_help = @{$self->{'opt_help'}};
    foreach (@opt_desc) {
        my %h = %$_;
        foreach my $key (keys %h) {
            #print "Key : $h{$key}\n";
            if ($key eq 'opt') {
                push @opt_vals, $h{'opt'};
            } elsif ($key eq 'desc') {
                my $option = $h{'opt'};
                $option =~ s/([^=]+)=.+/$1/;
                $option =~ s/([^|]+)/(length "$1" > 1 ? '--' : '-') . "$1"/eg;
                push @opt_help, [$option, $h{'desc'}];
            }
        }
    }
    $self->{'opt_help'} = \@opt_help;
    return @opt_vals;
}

# Parse command line options
sub get_opts {
    my $self = shift;
    my @user_opt_desc = @_;
   
    my @opt_desc = (
        {
            opt => 'help|h',
            desc => 'Display this help and exit.',
        },
        
        {
            opt => 'version|v',
            desc => 'Print version and exit.',
        },           
        
        {
            opt => 'file|F=s',
            desc => 'Input file (default: STDIN).',
        },            
        
        {
            opt => 'output|o=s',
            desc => 'Output file (default: STDOUT).',
        },
        
        {
            opt => 'id=s@',
            desc => 'svg id to operate on (can be multiple).',
        },           
        
        {   
            opt => 'ids=s',
            desc => 'Comma-separated list of svg ids to operate on.',
        },           
    );
 
    # Create option arrays for Getopt::Long
    my @opt_vals = $self->make_opt_vals(@opt_desc);
    my @user_opt_vals = $self->make_opt_vals(@user_opt_desc);
    
    # Append user options 
    foreach (@user_opt_vals) {
        push @opt_vals, $_;
    }
    
    # Where the parsed options are stored
    my %opts;

    #exit 0;

    # Parse all options
    GetOptions(\%opts, @opt_vals) or usage();    

    # Handle comma-separated 'ids=foo,bar'
    my @ids = @{$opts{'id'}} if $opts{'id'};
    if (exists $opts{'ids'} && $opts{'ids'} =~ /[\w\d_]+(,[\w\d_]+)*/) {
        push (@ids, split(/,/, $opts{'ids'}));
    }

    # Display usage etc. (and exit)
    exists $opts{'version'} && $self->version();
    exists $opts{'help'} && $self->usage(); 

    # Save id values for later processing 
    $self->{'ids'} = \@ids;
    
    # Save options
    $self->{'opts'} = \%opts;

    # Return the options to script
    return %opts;
}

# Exit with named exit status
sub error {
    my $self = shift;
    my $error_name = shift;
    my $script_error_msg = shift || '';
   
    my %status = %{$self->{'status'}};

    foreach (keys %status) {
        if ($status{$_}[0] eq $error_name) {
            $! = $_; # Set exit status

            # Commented out; let sodipodi handle the error code instead
            #my $msg =  ($status{$_}->[1] . ": $script_error_msg");
            
            my $msg =  "$script_error_msg";
            die $msg;
        }
    }
    
    # Will not be reached unless an improper error_name is given
    $! = 255; # Exit status 
    warn "Illegal error code '$error_name' called from script\n";
}

# Some accessor methods
sub set_usage {
    my $self = shift;
    my $usage = shift || die "No usage string supplied!\n";
    $self->{'usage'} = $usage;
}

sub set_name {
    my $self = shift;
    my $name = shift || die "No script name supplied!\n";
    $self->{'name'} = $name;
}

# Print usage and exit
sub usage {
    my $self = shift;
    print "Usage: $self->{'name'} OPTIONS FILE\n";
    print $self->{'usage'};
    
    my @opt_help = @{$self->{'opt_help'}};
    foreach (@opt_help) {
        print pad($_->[0]) . $_->[1] . "\n";
    }

    exit ARG_ERR(); 
}

sub pad {
    my $string = shift;
    my $width = '20';
    return $string . ' ' x ($width - length($string));
}

# Print version
sub version {
    print "Uses SpSVG version $VERSION\n";
    exit ARG_ERR();
}

# End of module; return something true
1;

__END__

DOCUMENTATION HERE
