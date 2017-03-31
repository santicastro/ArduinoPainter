// Wade's Extruder Gears using Parametric Involute Bevel and Spur Gears by GregFrost
// by Nicholas C. Lewis (A RepRap Breeding Program)
// Modified by Santiago Castro on 2013
// 
// It is licensed under the Creative Commons - GNU GPL license.
// © 2010 by Nicholas C. Lewis
// http://www.thingiverse.com/thing:4305

use <mendel_misc.inc>
use <parametric_involute_gear_v5.0.scad>
 $fn=60;
 
teethL = 38;
teethS = 22;

showL = true;
showS = false;

separated = false;

if(showL && showS){
	if(separated){
		translate([-50,0,0]) rotate([0,0,360/teethL/2]) WadesL();
	}else{
		translate([-45,0,0]) rotate([0,0,360/teethL/2]) WadesL();
	}
}else if(showL){
	WadesL();
}
if(showS){
	WadesS();
}


module WadesL(){
	//Large WADE's Gear
	//rotate([0,0,-2])translate([0,0,0])color([ 100/255, 255/255, 200/255])import_stl("39t17p.stl");
	 rim_thickness = 6;
	gear_thickness = 5;
	hub_thickness = 13;
	difference(){
		gear (number_of_teeth=teethL,
			circular_pitch=268,
			gear_thickness = gear_thickness,
			rim_thickness = rim_thickness,
			rim_width = 3,
			hub_thickness = hub_thickness,
			hub_diameter = 20,
			bore_diameter = 5.2,
			circles=7);
	      translate([0,-3.9,rim_thickness+(hub_thickness-rim_thickness)/2])rotate([90, 0, 0])rotate([0, 0, 30])cylinder(r=3.05+0.15, h=2.3,$fn=6);
	//	translate([0,0,6])rotate([180,0,0])m8_hole_vert_with_hex(100);
		translate([0,-5,15.5])cube([5.8,2.3,9.5],center = true);
		translate([0,0,9])rotate([0,90,-90])cylinder(r=1.7,h=20);
	}
}

module WadesS(){
	//small WADE's Gear
	//rotate([180,0,-23.5])translate([-10,-10,-18])color([ 100/255, 255/255, 200/255])import_stl("wades_gear.stl");
    rim_thickness = 8;
	gear_thickness = 8;
	hub_thickness = 15;
	difference(){
		gear (number_of_teeth=teethS,
			circular_pitch=268,
			gear_thickness = gear_thickness,
			rim_thickness = rim_thickness,
			hub_thickness = hub_thickness,
			hub_diameter = 20,
			bore_diameter = 5.2,
			circles=0);
        translate([0,-3.9,rim_thickness+(hub_thickness-rim_thickness)/2])rotate([90, 0, 0])rotate([0, 0, 30])cylinder(r=3.05+0.15, h=2.3,$fn=6);
	// screw
		translate([0,-5,15.5])cube([5.8,2.3,9.5],center = true);
		translate([0,0,rim_thickness+(hub_thickness-rim_thickness)/2])rotate([0,90,-90])cylinder(r=1.7,h=20, $fn=30);
	}
}
