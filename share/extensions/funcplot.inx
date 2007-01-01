<inkscape-extension>
    <_name>Function Plotter</_name>
    <id>org.inkscape.effect.funcplot</id>
	<dependency type="executable" location="extensions">funcplot.py</dependency>
	<dependency type="executable" location="extensions">inkex.py</dependency>
	<param name="tab" type="notebook">
	    <page name="sampling" _gui-text="Range and Sampling">
	        <param name="xstart" type="float" min="-1000.0" max="1000.0" _gui-text="Start x-value">0.0</param>
	        <param name="xend" type="float" min="-1000.0" max="1000.0" _gui-text="End x-value">1.0</param>
         	<param name="times2pi" type="boolean" _gui-text="Multiply x-range by 2*pi">false</param>
	        <param name="ybottom" type="float" min="-1000.0" max="1000.0" _gui-text="y-value of rectangle's bottom">0.0</param>
	        <param name="ytop" type="float" min="-1000.0" max="1000.0" _gui-text="y-value of rectangle's top">1.0</param>
	        <param name="samples" type="int" min="2" max="1000" _gui-text="Samples">8</param>
            <param name="isoscale" type="boolean" _gui-text="Isotropic scaling (uses smallest: width/xrange or height/yrange)">false</param>
	    </page>
	    <page name="desc" _gui-text="Help">
        	<_param name="pythonfunctions" type="description">The following functions are available:
(the available functions are the standard python math functions)
ceil(x); fabs(x); floor(x); fmod(x,y); frexp(x); ldexp(x,i); 
modf(x); exp(x); log(x [, base]); log10(x); pow(x,y); sqrt(x); 
acos(x); asin(x); atan(x); atan2(y,x); hypot(x,y); 
cos(x); sin(x); tan(x); degrees(x); radians(x); 
cosh(x); sinh(x); tanh(x).

The constants pi and e are also available. </_param>
	    </page>
	</param>
	<param name="fofx" type="string" _gui-text="Function">exp(-x*x)</param>
	<param name="fponum" type="boolean" _gui-text="Calculate first derivative numerically">true</param>
	<param name="fpofx" type="string" _gui-text="First derivative">x</param>
	<param name="remove" type="boolean" _gui-text="Remove rectangle">true</param>
    <param name="drawaxis" type="boolean" _gui-text="Draw Axes">false</param>
    <effect>
		<object-type>rect</object-type>
                <effects-menu>
                        <submenu _name="Render"/>
                </effects-menu>
    </effect>
    <script>
        <command reldir="extensions" interpreter="python">funcplot.py</command>
    </script>
</inkscape-extension>
