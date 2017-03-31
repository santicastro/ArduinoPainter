seno = sin(2.4)*16;
module etch(weight=20) {
  etch_length=187.6;
  linear_extrude(height = weight)
  polygon( points=[[0, 3],[4,3],[4,0],[etch_length-4, 0],[etch_length-4,3],[etch_length,3],[etch_length-seno, 3+16],[etch_length-seno-7, 3+16+7],[seno+7, 3+16+7],[seno, 3+16]] );
  translate([seno+9, 3+16-2])cylinder(r=9, h=weight);
  translate([etch_length-(seno+9), 3+16-2])cylinder(r=9, h=weight);
}

module nema14support(height=5) {
  $fn=60;//translate([12, 0])rotate([0,0,45])
  union() {
    difference() {
      hull() {
        translate ([0,0,0]) translate([-6, -6])cube([12, 12, height]);//cylinder (r=4.6,h=height);
        translate ([26,0,0]) translate([-6, -6])cube([12, 12, height]);//cylinder (r=4.6,h=height);
        translate ([0,26,0]) cylinder (r=6,h=height);
        translate ([26,26,0]) cylinder (r=6,h=height);
      }
      
      translate ([0,0,-0.5]) cylinder (r=1.7,h=height+1);
      translate ([26,0,-0.5]) cylinder (r=1.7,h=height+1);
      translate ([0,26,-0.5]) cylinder (r=1.7,h=height+1);
      translate ([26,26,-0.5]) cylinder (r=1.7,h=height+1);
      
      translate ([13,13,-0.5]) cylinder (r=12,h=height+1);
      
      hull() {
        translate ([13,13,-0.5]) cylinder (r=12,h=height+1);
        translate ([-4,13,-0.5]) cylinder (r=0.1,h=height+1);
      }
    }
    translate([-4.6, 3])cube([5, 20, height]);
  }
  
}
module support(height = 1) {
  $fn=60;
  module part(distance=80, bigradius=78.8, smallradius=12, height = height, square_length=50) {
    difference() {
      union() {
        cylinder(r=smallradius, h=height);
        translate([0, -10, 0])cube([distance, square_length,height]);
        translate([distance, 0, 0])cylinder(r=smallradius, h=height);
      }
      translate([distance/2, -distance-1, -height/2])cylinder(r=bigradius, h=height*2);
    }
  }
  difference() {
    union() {
      part(80, 78.8);
      mirror([1,0])rotate([0, 0, 90])part(120, 123);
      translate([80, 0])rotate([0, 0, 90])part(120, 123);
    }
    union() {
      height = 4.5;
      translate([2, 2, -0.01])part(76, 79.5, 7, height, 25);
      mirror([1,0])translate([-2, 2, -0.01])rotate([0, 0, 94])part(76, 79.5, 7, height, 25);
      translate([76, 0])translate([2, 2, -0.01])rotate([0, 0,88])part(76, 79.5, 7, height, 25);
      translate([10, 14, -0.01])cube([60, 30, height]);
    }
    
  }
  
}
mirror([1, 0])rotate([0, 0, 90])
rotate([180, 0, 90])rotate([0, 0, 42])
union() {
  //cuerpo
  difference() {
    weight = 42;
    union() {
      hull() {
        translate([8, 8])cylinder(r=8, h=weight);
        translate([8, 8+26])cylinder(r=8, h=weight);
        translate([5+8, 8])cylinder(r=8, h=weight);
        translate([5+8, 8+26])cylinder(r=8, h=weight);
      }
      translate([183,0])hull() {
        translate([8, 8])cylinder(r=8, h=weight);
        translate([8, 8+22])cylinder(r=8, h=weight);
        translate([5+8, 8])cylinder(r=8, h=weight);
        translate([5+8, 8+22])cylinder(r=8, h=weight);
      }
      
      hull() {
        translate([8, 8])cylinder(r=8, h=weight);
        translate([8+160+35-7, 8])cylinder(r=8, h=weight);
      }
    }
    color("black")translate([10, 20, 42.6])rotate([0, 45, 0])cube([20, 40, 20]);
    color("black")translate([10+159.4, 20, 42.6])rotate([0, 45, 0])cube([20, 40, 20]);
    color("black")translate([180, 40, 18])rotate([90, 0, 0])cylinder(r=12, h=20);
    translate([10, 10, -0.1])etch(60);
    color("blue") hull() {
      translate([12, -1.5, -1])cube([180, 0.01, 30]);
      translate([28+9, 2.5, -1])cube([148-18, 0.1, 30]);
    }
    color("blue") hull() {
      translate([12, 11.5, -1])cube([180, 0.01, 30]);
      translate([28+9, 7.5, -1])cube([148-18, 0.01, 30]);
    }
  }
  nemaHeight = 5;
  // nema support
  translate([210, 36, 32])rotate([0, 90, -90]) union() {
    translate([26, 0])mirror([1, 0])nema14support(nemaHeight);
    translate([-6, -12])cube([38, 10, nemaHeight]);
  }
  // nema support support
  translate([200, 31, 42-4]) {
    hull() {
      cube([36, nemaHeight, 4]);
      translate([-1, -28])cylinder(r=2, h=4);
    }
    translate([33.5, 0, 2])rotate([0, 52, 0])cube([6.5, 5, 3]);
  }
  
  //%translate([206, -4, 2])color("blue")cube([34,34,34]);
  
  // support
  difference() {
    translate([104, -44, 32])rotate([0, 0, 20])support(10);
    cube([200, 120, 50]);
    
    //translate([114, -46])rotate([0, 0, 20])scale([0.15, 0.15, 1])linear_extrude(h=10)rotate([0, 0, 12])import("/Users/Santi/Desktop/bricolabs_logo.dxf");
    translate([115, -59])rotate([0, 0, 18])scale([0.2, 0.2, 1])linear_extrude(h=10)rotate([0, 0, 12])import("../bricolabs_logo_sin_letra_inverse.dxf");
  }
 translate([116.5, -10.5, 36.5])rotate([0, 0, 8])cube([2,10,2]);
 translate([117-9.5, -10.6-5.5, 36.5])rotate([0, 0, 50])cube([2,10,2]);
 translate([109.5, -36.3, 36.5])rotate([0, 0, 128])cube([2,10,2]);
 translate([144.5, -18.3, 36.5])rotate([0, 0, 106])cube([2,10,2]);
  
  //color("blue")translate([124, -72, 32])rotate([0, 0, 20])scale([0.35, 0.35, 1])linear_extrude(height=10)rotate([0, 0, 0])import("../bricolabs_logo_sin_letra.dxf");
  // border
  translate([10, 0, 35])cube([186, 13, 7]);
}

//translate([0, -28])color("violet")cube([190, 190, 0.1]);

