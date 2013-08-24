// Wade's Extruder Gears using Parametric Involute Bevel and Spur Gears by GregFrost
// by Nicholas C. Lewis (A RepRap Breeding Program)
// Modified by Santiago Castro on 2013
// 
// It is licensed under the Creative Commons - GNU GPL license.
// © 2010 by Nicholas C. Lewis
// http://www.thingiverse.com/thing:4305

use <mendel_misc.inc>
use <parametric_involute_gear_v5.0.scad>

teethL = 39;
teethS = 20;

showL = true;
showS = true;

if(showL && showS){
	translate([-48,0,0]) rotate([0,0,360/teethL/2]) WadesL();
}else if(showL){
	WadesL();
}
if(showS){
	WadesS();
}


module WadesL(){
	//Large WADE's Gear
	//rotate([0,0,-2])translate([0,0,0])color([ 100/255, 255/255, 200/255])import_stl("39t17p.stl");
	difference(){
		gear (number_of_teeth=teethL,
			circular_pitch=268,
			gear_thickness = 5,
			rim_thickness = 7,
			rim_width = 3,
			hub_thickness = 13,
			hub_diameter = 20,
			bore_diameter = 5,
			circles=7);
	//	translate([0,0,6])rotate([180,0,0])m8_hole_vert_with_hex(100);
		translate([0,-5,12])cube([5.5,2.3,9],center = true);
		translate([0,0,10])rotate([0,90,-90])cylinder(r=1.7,h=20);
	}
}

module WadesS(){
	//small WADE's Gear
	//rotate([180,0,-23.5])translate([-10,-10,-18])color([ 100/255, 255/255, 200/255])import_stl("wades_gear.stl");
	difference(){
		gear (number_of_teeth=20,
			circular_pitch=268,
			gear_thickness = 9,
			rim_thickness = 9,
			hub_thickness = 18,
			hub_diameter = 20,
			bore_diameter = 5,
			circles=0);
	// screw
		translate([0,-5,16])cube([5.5,2.3,9],center = true);
		translate([0,0,14])rotate([0,90,-90])cylinder(r=1.7,h=20);
	}
}
