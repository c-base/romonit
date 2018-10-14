/* model for the romonit sensor enclosure
vim: ai ts=2
*/
$fn=20;
angle=10;
wall=1.7;
faceheight=76;
innerwidth=30;
innerdepth=23;

use <romonit.scad>

module foot(ro, ri, h) {
	difference() {
		cylinder(r=ro, h=h);
		translate([ 0, 0, -0.1]) cylinder(r=ri, h=h+0.2);
	}
}

module feet(ro, ri, h) {
	// move the feet in the position they need on the romonit
	//translate([2, 41.5, 1.6])
	translate([ro, ro, 0])
	union() {
		translate([    0,    0, 0]) foot(ro,ri,h);
		translate([    0, 29.6, 0]) foot(ro,ri,h);
		translate([ 23.1,    0, 0]) foot(ro,ri,h);
		translate([ 23.1, 29.6, 0]) foot(ro,ri,h);
	}
}

module outer_box(w, d, h) {
	w = w + 2*wall;
	foffs = wall / tan(90-angle) + wall / sin(90-angle);
	idepth  = sqrt( pow(h,2) - pow((cos(angle) * h),2) );
	odepth = idepth + foffs;
	iheight = sqrt( pow(h,2) - pow((sin(angle) * h),2) );
	oheight = iheight + 2*wall;
	face = sqrt( pow(oheight,2) + pow(odepth,2) );

	difference() {
		cube([w, odepth+d, oheight]);
		translate([-1,0,0]) rotate([90-angle,0,0]) cube([w+2,face,odepth]);
		//window 25mm wide, 22mm high, offset 64.5mm from top
		rotate([90-angle,0,0]) translate([(w-25)/2,faceheight-64+2.5+wall,-wall-0.5]) cube([25,22,wall+2]);
	}

}

module inner_box(w, d, h) {
	foffs  = wall / tan(90-angle) + wall / sin(90-angle);
	depth  = sqrt( pow(h,2) - pow((cos(angle) * h),2) );
	height = sqrt( pow(h,2) - pow((sin(angle) * h),2) );
	translate([wall,foffs,wall])
	difference() {
		cube([w, depth + d + 1, height]);
		translate([-1,0,0]) rotate([90-angle,0,0]) cube([w+2,h,depth]);
		// feet are 27.1mm wide and 33.6mm "high"
		rotate([-angle,0,0])  translate([(w-27.1)/2 , 9.2, h-33.6]) rotate([90,0,0]) feet(2,0.6,9.2);
	}
	// holes 1.5mm diameter
	rad=0.75;
	//border 5mm
	b=3;
	ssteps=floor( (h-2*b) /(4*rad) );
	sstep=(h-2*b)/ssteps;
	bsteps=floor( (w-2*b) /(4*rad) );
	bstep=(w-2*b)/bsteps;
	translate([wall,foffs,wall])
	union() {
		// holes in side walls
		for(y = [b:sstep:depth+d-5]) {
			for(z = [b:sstep:height-b]) {
				assign(offs =  z * tan(angle) ) {
					//echo(y, offs);
					if ( y > (offs + b) ){
						translate([w,y,z]) rotate([90, 0,90]) cylinder(r=rad, h=wall+0.2);
						translate([-wall-0.1,y,z]) rotate([90, 0,90]) cylinder(r=rad, h=wall+0.2);
					}
				}
			}
		}
		for(y = [b:bstep:depth+d-5]) {
			for(x = [b:bstep:w-b]) {
				translate([x,y,-wall-0.1]) rotate([ 0, 0, 0]) cylinder(r=rad, h=wall+0.2);
				if ( y >= depth+b ) {
					translate([x,y,-0.1+height]) rotate([ 0, 0, 0]) cylinder(r=rad, h=wall+0.2);
				}
			}
		}
	}
}

module box(w, d, h) {
	rotate([90+angle,0,90])
	difference() {
		outer_box(w,d,h);
		inner_box(w,d,h);
	}
	//rotate([90-angle,0,0]) translate([ (innerwidth-27.2)/2+wall, faceheight-73+wall+wall*sin(angle), (-9.2-1.6-wall) ]) romonit();
}

module back(w,d,h) {
	foffs = wall / tan(90-angle) + wall / sin(90-angle);
	height = sqrt( pow(h,2) - pow((sin(angle) * h),2) );
	depth  = sqrt( pow(h,2) - pow((cos(angle) * h),2) );
	//translate([wall+0.1,foffs+depth+d-4+0.1,wall+0.1])
	translate([0, -10, 4]) rotate([90,180,90])
	union() {
		difference() {
			cube([w-0.2,4,height-0.2]);
			translate([wall, -0.1, wall]) cube([w-2*wall-0.1,4-wall+0.1,height-2*wall-0.1]);
			translate([w/2, 0, height-15]) rotate([90, 90, 0]) union() {  translate([0,0,-5]) cylinder(r=5,h=4); translate([-8,-2,-5]) cube([8,4,4]); translate([-8,0,-5]) cylinder(r=2,h=4); }
		}
		translate([ 0, -4, 1]) cube([wall, 4, 6]);
		translate([ 0, -4, height-6-2]) cube([wall, 4, 6]);
		translate([ w-wall-0.2, -4, 1]) cube([wall, 4, 6]);
		translate([ w-wall-0.2, -4, height-6-2]) cube([wall, 4, 6]);
	}
}

box(innerwidth,innerdepth,faceheight);
back(innerwidth,innerdepth,faceheight);

