// esp8266-si2071 case
include <threads.scad> // https://dkprojects.net/openscad-threads/


wall_thickness	= 2.0;   // minimum 1.0

pcb_w = 34;   // PCB width
pcb_l = 50; // PCB length
pcb_h = 8; // PCB height, assembled
batt_w = 38;  // Battery width
batt_l = 55;  // Battery length
batt_h = 6;  // Battery height

overall_w = max(batt_w, pcb_w) + 2*wall_thickness;
overall_l = max(batt_l, pcb_l) + 2*wall_thickness;
overall_h = batt_h + pcb_h + 2*wall_thickness;

echo(overall_w, overall_l, overall_h);

//difference() {
//    cube([overall_w,overall_l,overall_h]); // width, depth, height
//    cube([overall_w - 2*wall_thickness,overall_l - 2*wall_thickness,overall_h-2*wall_thickness]); // width,
//};


//$fn=50; // number of vertices
//minkowski() // add following
//{
//  cube([10,10,1]);
//  cylinder(r=2,h=1);
//}

//$fn=50;
//difference() {
//    cylinder(r=2, h=2);
//    translate(v=[0,0,-0.1]) {
//        metric_thread(3, 0.35, 2.2, internal=true);
//    }
//}

// draw bottom
difference() {
    cube([overall_w,overall_l,overall_h/2]); // width, depth, height
    cube([overall_w - 2*wall_thickness,overall_l - 2*wall_thickness,(overall_h)]); // width,
};


