/* model of the romonit sensor for enclosure design
vim: ai ts=2
*/

$fn=50;
r = 0.85;

module romonit() {
	// PCB
	difference() {
		color("ForestGreen",1.0) cube([27,73,1.6],center=false);
		union() {
			// edge connector left cut
			translate([ 8.4,-r,-0.1]) union() {
				translate([r,r,0]) minkowski() {
					cube([2.5-2*r,6.4-r,1.7]);
					cylinder(r=r,h=1);
				}
			}
			// edge connector right cut
			translate([20.0,-r,-0.1]) union() {
				translate([r,r,0]) minkowski() {
					cube([2.5-2*r,6.4-r,1.7]);
					cylinder(r=r,h=1);
				}
			}
			// battery cut
			translate([8.6,40,0]) union () {
				translate([r,r,0]) minkowski() {
					cube([14.6-(2*r),25.6-(2*r),1.7]);
					cylinder(r=r,h=1,center=true);
				}
			}
			// holes
			translate([ 1.85 ,41.25,-0.1]) cylinder(r=0.85,h=1.8);
			translate([25.05 ,41.25,-0.1]) cylinder(r=0.85,h=1.8);
			translate([ 1.85 ,71.10,-0.1]) cylinder(r=0.85,h=1.8);
			translate([25.05 ,71.10,-0.1]) cylinder(r=0.85,h=1.8);
		}
	}

	// LCD with pins
	union () {
		// LCD
		translate([-0.1,9.2,7.75]) color("SlateGray") cube([27.2,26.4,2.75]);
		// lower pins
		for (i=[0:9]) {
			translate([5.2+1.78*i,9.1,-0.5]) color("WhiteSmoke") cube([0.4,0.3,9.9]);
		}
		// upper pins
		for (i=[0:7]) {
			translate([7.1+1.78*i,35.5,-0.5]) color("WhiteSmoke") cube([0.4,0.3,9.9]);
		}
	}

	// shrouded header
	translate([0,43.7,1.5]) difference() {
		// outer box
		color("Black") cube([8.6,25.2,9.1]);
		// inner box
		translate([1,1,2]) cube([6.9,23.2,7.3]);
	}

	// push button
	translate([10.4,66,1.5]) union() {
		color("DarkSlateGray")
		cube([3.8,6.5,5.0]);
		translate([1.9,4.8,5]) cylinder(r=1,h=1);
		translate([1.9,4.6,5]) cube([1.7,0.4,1]);
		translate([1.9,4.8,6]) cylinder(r=0.5,h=5.6);
	}

	// battery
	translate([15.8,40.4,-0.35]) rotate([-90,0,0]) color("Gold") cylinder(r=7.15,h=24.8);
}

romonit();

