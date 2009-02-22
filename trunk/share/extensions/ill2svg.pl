#!/usr/bin/perl
# convert an illustrator file (on stdin) to svg (on stdout)
use strict;
use warnings;
use Getopt::Std;

my $skip_images;

BEGIN {
  $skip_images = 0;
  eval "use Image::Magick;";
  if ($@) {
    warn "Couldn't load Perl module Image::Magick.  Images will be skipped.\n";
    warn "$@\n";
    $skip_images = 1;
  }
}


my %args;

# Newline characters
my $NL_DOS = "\015\012";
my $NL_MAC = "\015";
my $NL_UNX = "\012";

getopts('h:', \%args);

my @ImageData;
my $pagesize=1052.36218;
my $imagewidth = 128;
my $imageheight = 88;
my $imagex = 0;
my $imagey = 0;
my $imagenum = 0;
my $strokeparams;
my $strokecolor;
my $strokewidth;
my $fillcolor;
my $firstChar = 0;
my $image;
my $path;
my $color = 0;
my $weareinimage=0;
my $weareintext=0;
my($red,$green,$blue);
my($cpx,$cpy);

if ($args{h}) { usage() && exit }

$color = "#000";

sub addImageLine {
 my ($data) = @_;
 chomp($data);
 #push (@ImageData, $data);
  
 my $len = length( $data );
 my $count = 0;
 
 #printf("%s %d\r\n",$data,$len);
 #for( $loop=1; $loop < ($len - 2); $loop += 2){
 for( my $loop=1; $loop < $len; $loop += 2){
 	my $value = substr( $data, $loop, 2);
	
	#printf("[%d:%s]",$loop,$value);
         
        if( $color == 0 ){
          #$red = hex($value);
          $red = $value;
	  $color ++;
          #printf("Color: RED: %s,",$red);
	} elsif ( $color == 1 ) {
          #$green = hex($value);
          $green = $value;
	  $color ++;
          #printf("Color: GREEN %s,",$green);

        } else {
	  $blue = $value;
	  my $pixel="pixel[${imagex},${imagey}]";
	  my $rgb=sprintf("#%s%s%s",$red,$green,$blue);
	  #$image->Set($pixel=>$rgb);                    
	  $image->Set("pixel[${imagex},${imagey}]"=>"#${red}${green}${blue}")
		unless ($skip_images);
	  #printf("PIXX: %d ",$image->Get($pixel));
	  $color = 0;
          $imagex++;

          #printf("Color:BLUE: %s, X: %d Y: %d %dx%d  %s [%s]\r\n",$blue,$imagex,$imagey,$imagewidth,$imageheight,$rgb,$pixel); 

         if( $imagex == $imagewidth ){
          $imagex = 0;
          $imagey ++; 
         } # if


        } #else

 }

 #printf("len: %d %dx%d\r\n",$len,$imagewidth,$imageheight);

}

sub cmyk_to_css {
    my ($c, $m, $y, $k) = @_;
    my ($r, $g, $b);

    $r = 1 - ($k + $c);
    if ($r < 0) { $r = 0; }
    $g = 1 - ($k + $m);
    if ($g < 0) { $g = 0; }
    $b = 1 - ($k + $y);
    if ($b < 0) { $b = 0; }
    return sprintf ("#%02x%02x%02x", 255 * $r, 255 * $g, 255 * $b);
}

sub nice_float {
    my ($x) = @_;

    my $result = sprintf ("%.3f", $x);
    $result =~ s/0*$//;
    $result =~ s/\.$//;
    return $result;
}

sub xform_xy {
    my ($x, $y) = @_;
    my @result = ();
    
    for my $i (0..$#_) {
	if ($i & 1) {
	    #push @result, 1000 - $_[$i];
	    push @result, $pagesize - $_[$i];
	} else {
	    #push @result, $_[$i] - 100;
	    push @result, $_[$i];
	}
    }
    return join ' ', map { nice_float ($_) } @result;
}

sub strokeparams {
    $strokecolor ||= 'black';
    my $result = "stroke:$strokecolor";
    if ($strokewidth && $strokewidth != 1) {
	$result .= "; stroke-width:$strokewidth";
    }
    return $result;
}

sub usage {
    warn qq|Usage: ill2svg [-l "string" -h] infile > outfile
options: 
	-h print this message and exit
|;
}

sub process_line {
    chomp;
    return if /^%_/;
      
    if (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) k$/) {
	$fillcolor = cmyk_to_css ($1, $2, $3, $4);
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) K$/) {
	$strokecolor = cmyk_to_css ($1, $2, $3, $4);
    } elsif (/^([\d\.]+) g$/) {
	$fillcolor = cmyk_to_css (0, 0, 0, 1 - $1);
    } elsif (/^([\d\.]+) G$/) {
	$strokecolor = cmyk_to_css (0, 0, 0, 1 - $1);
    } elsif (/^([\d\.]+) ([\d\.]+) m$/) {
	$path .= 'M'.xform_xy($1, $2);
	$cpx = $1;
	$cpy = $2;
    } elsif (/^([\d\.]+) ([\d\.]+) l$/i) {
	$path .= 'L'.xform_xy($1, $2);
	$cpx = $1;
	$cpy = $2;
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) v$/i) {
	$path .= 'C'.xform_xy($cpx, $cpy, $1, $2, $3, $4);
	$cpx = $3;
	$cpy = $4;
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) y$/i) {
	$path .= 'C'.xform_xy($1, $2, $3, $4, $3, $4);
	$cpx = $3;
	$cpy = $4;
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) c$/i) {
	$path .= 'C'.xform_xy($1, $2, $3, $4, $5, $6);
	$cpx = $5;
	$cpy = $6;
    } elsif (/^b$/) {
	$path .= 'z';
	$strokeparams = strokeparams ();
	print " <g style=\"fill: $fillcolor; $strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^B$/) {
	$strokeparams = strokeparams ();
	print " <g style=\"fill: $fillcolor; $strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^f$/i) {
	$path .= 'z';
        if (! $fillcolor) {
            warn "Error:  Fill color not defined in source file!\n";
            print "  <path d=\"$path\"/>\n";
        } else {
            print " <g style=\"fill: $fillcolor;\">\n";
            print "  <path d=\"$path\"/>\n";
            print " </g>\n";
        }
	$path = '';
    } elsif (/^s$/) {
	$path .= 'z';
	$strokeparams = strokeparams ();
	#print " <g style=\"fill:none;stroke:black;stroke-opacity:1; $strokeparams\">\n";
	print " <g style=\"fill:none; $strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^S$/) {
	$strokeparams = strokeparams ();
	#print " <g style=\"fill:none; $strokeparams\">\n";
	print " <g style=\"fill:none;stroke:black;$strokeparams;\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^1 XR$/) {
       
       if( $firstChar != 0){
         print (" </tspan>\n</text>\n");
       }
      
       $weareintext=0;

       $firstChar = 0;	
    } elsif (/^TP$/) {
       #Something do with the text;)
#      $weareintext=1;
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) Tp$/i) {
 
       # Text position etc;
       $cpx=$5;
       $cpy=$pagesize - $6;         
       ## print ("x:$5 y:$6\r\n");
       
    } elsif (/^TO$/) {
      #one text ends       
    } elsif (/^LB$/) {
      #Everything ends??
      ## Sometimes ain't working
      warn "Error:  Unexpected 'LB'!  Everything has ended??\n";
      if( $weareintext != 0){
         print ("  </tspan>\n</text>\n");
      } 
    
    } elsif (/^\/_([\S\s]+) ([\d\.]+) Tf$/) {
       my $FontName = $1;
       my $FontSize = $2;
      
       ## When we know font name we can render this.
       if( $firstChar != 1){
	 print ("<text x=\"$cpx\" y=\"$cpy\"");  
         print (" style=\"font-size:$FontSize;font-weight:normal;stroke-width:1;");
         print ("fill:$fillcolor;fill-opacity:1;");
         print ("font-family:$FontName;\" id=\"text$5\">\n<tspan>");
	 $firstChar = 1;
         $weareintext=1;
       } else {
	 print ("</tspan>\n<tspan x=\"$cpx\" y=\"$cpy\"");
	 print (" style=\"font-size:$FontSize;font-weight:normal;stroke-width:1;");
         print ("fill:$fillcolor;fill-opacity:1;");
         print ("font-family:$FontName;\" id=\"text$5\">");
         $weareintext=1;
       }

    } elsif (/^\(([\S\s]+)\) Tx$/) {
        # Normal text
 	my $text = $1;
        $text =~ s/ä/Ã¤/;
        $text =~ s/ö/Ã¶/;
        $text =~ s/å/Ã¥/;
	
	print ("$text");
   
   } elsif (/([\d\.]+) w$/) {
	        $strokewidth = $1;
    } elsif (/^%%BeginData: ([\d\.]+)/) {
          #How much we got image data
    } elsif(/^%%EndData/) {
	  #and the data ends
    } elsif (/^XI/) {
          #actual image starts
          $weareinimage=1;
    } elsif (/^XH/) {
          #it ends like they allways do.
	  #we just save it after this..
          $weareinimage=0;
	  my $imagename="${imagenum}.png";
	  $image->Write($imagename) unless ($skip_images);
          $imagenum++;       
    } elsif (/\[(.*)\](.*)Xh/) {
	   my @imagepos = split(/ /, $1);
	   my @imageinfo = split(/ /, $2);
           $imagewidth = $imageinfo[1];
           $imageheight = $imageinfo[2];

           if (! $imagewidth && ! $imageheight ) {
               die "ERROR:  This fileformat is not supported by "
                   ."ill2svg.pl.\n(Is it possible you are trying to "
                   ."use ill2svg.pl on a PDF file?) \n";
	   }

           my $imageposx = $imagepos[4];
           my $imageposy = $pagesize - $imagepos[5];

	   #printf("%s %d %d Position x: %f 9y: %f\r\n",$1,@imagepos[4],@imagepos[5],$imageposx,$imageposy);
	   printf("<image");
	   printf(" xlink:href=\"%d.png\"",$imagenum);
           printf(" width=\"%d\"",$imagewidth);
	   printf(" height=\"%d\"",$imageheight);
           printf(" x=\"%f\"",$imageposx);
           printf(" y=\"%f\"",$imageposy);  
	   printf("/>\r\n");
	   
	   my $size = "${imagewidth}x${imageheight}";

           if (!$skip_images) {
             $image = Image::Magick->new(size=>$size);
	     $image->Set('density'=>'72');
	     $image->Read("xc:white");
           }
	   $imagex=0;
	   $imagey=0;
    
    } else {
         if( $weareinimage == 1 ){
            addImageLine( $_ );
         }  
 
         chomp;

         #print "$_\r\n";
    }
}
    if( $firstChar != 0){
       print ("   </tspan>\n</text>\n");
    }

print "<svg";
print " xmlns=\"http://www.w3.org/2000/svg\"";
print " xmlns:xlink=\"http://www.w3.org/1999/xlink\">\r\n";

while (<>) {
    foreach (split /[\015\012]+/) {
	process_line($_);
      }
}
print "</svg>\n";

