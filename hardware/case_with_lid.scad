// Todo, the rotation of the lid makes the lid vent alignment hard-coded

wallThickness=2;

pcb_w = 34;   // PCB width
pcb_l = 50; // PCB length
pcb_h = 8; // PCB height, assembled
batt_w = 38;  // Battery width
batt_l = 51;  // Battery length
batt_h = 6;  // Battery height
overall_w = max(batt_w, pcb_w) + 2*wallThickness;
overall_l = max(batt_l, pcb_l) + 2*wallThickness;
overall_h = batt_h + pcb_h + 2*wallThickness;

itemsShown="both"; // [both,box,lid]
boxLength=overall_l;
boxWidth=overall_w;
boxHeight=overall_h;
cornerRadius=5;
bottomThickness=2;
lidThickness=2;
lidClearance=0.2;
lidEdgeThickness=0.5;
// Notch in the lid
withNotch=true;//true;//[true:false]

sensor_pos_x = 24; // Sensor position
sensor_pos_lid_x = (overall_l - 1.5*sensor_pos_x) + wallThickness + lidEdgeThickness;

/* [Global] */
if (itemsShown=="box") showBox();
if (itemsShown=="lid") showLid();
if (itemsShown=="both"){showBox();showLid();}

module showLid(){
	translate ([0, -2*wallThickness, 0]) 
	roundBoxLid(l=boxLength-wallThickness,
				w=boxWidth-2*wallThickness-lidClearance,
				h=lidThickness,
				et=lidEdgeThickness,
				r=cornerRadius-wallThickness,
				notch=withNotch);
}

module showBox(){
	round_box(l=boxLength,
			  w=boxWidth,
			  h=boxHeight,
			  bt=bottomThickness,
			  wt=wallThickness,
			  lt=lidThickness,
			  r=cornerRadius);
}

module round_box(l=40,w=30,h=30,bt=2,wt=2,lt=2,r=5,){
	difference() { 
		round_cube(l=l,w=w,h=h-lt,r=r);
		translate ([wt, wt, bt]) 
		round_cube(l=l-wt*2,w=w-wt*2,h=h,r=r-wt);
        // air holes
        translate([sensor_pos_x,0,7]) cube([10,3,1]);
        translate([sensor_pos_x,0,9]) cube([10,3,1]);
        translate([sensor_pos_x,0,11]) cube([10,3,1]);
        translate([sensor_pos_x,0,13]) cube([10,3,1]);        
        translate([sensor_pos_x,boxWidth-(2*wallThickness),7]) cube([10,5,1]);
        translate([sensor_pos_x,boxWidth-(2*wallThickness),9]) cube([10,5,1]);
        translate([sensor_pos_x,boxWidth-(2*wallThickness),11]) cube([10,5,1]);
        translate([sensor_pos_x,boxWidth-(2*wallThickness),13]) cube([10,5,1]);
        translate([-wallThickness,boxWidth/2,7]) cube([5,10,1]);
        translate([-wallThickness,boxWidth/2,9]) cube([5,10,1]);
        translate([-wallThickness,boxWidth/2,11]) cube([5,10,1]);
        translate([-wallThickness,boxWidth/2,13]) cube([5,10,1]);
        translate([boxLength-wallThickness,boxWidth/2,7]) cube([5,10,1]);
        translate([boxLength-wallThickness,boxWidth/2,9]) cube([5,10,1]);
        translate([boxLength-wallThickness,boxWidth/2,11]) cube([5,10,1]);
        translate([boxLength-wallThickness,boxWidth/2,13]) cube([5,10,1]);
	}       


	//use two box rims. one to make a slope to support the lid
	roundBoxRim();
	translate ([0, 0, -wt]) roundBoxRim();
}

module roundBoxRim(l=boxLength,
				   w=boxWidth,
				   h=boxHeight,
				   et=lidEdgeThickness,
				   r=cornerRadius,
				   wt=wallThickness,
				   lt=lidThickness){
	difference() { 
		translate ([0, 0, h-lt]) 
		round_cube(l=l,w=w,h=lt,r=r);
		translate ([wt+lt,wt+lt-et*2,h-lt-0.1]) 
		round_cube(l=l*2,w=w-2*(wt+lt)+4*et,h=lt+0.2,r=r-wt+lt);

		//subtract out a lid to make the ledge
		translate ([wt, w-wt, h-lt-0.1])
		roundBoxLid(l=l*2,w=w-2*wt,h=lt+0.1,wt=wt,t=lt,et=0.5,r=r-wt,notch=false);
	}
}

module roundBoxLid(l=40,w=30,h=3,wt=2,t=2,et=0.5,r=5,notch=true){
	translate ([l, 0, 0]) 
	rotate (a = [0, 0, 180]) 

	difference(){

		round_cube(l=l,w=w,h=h,t=t,r=r);

		translate ([-1, 0, et]) rotate (a = [45, 0, 0])  cube (size = [l+2, h*2, h*2]); 
		translate ([-1, w, et]) rotate (a = [45, 0, 0])  cube (size = [l+2, h*2, h*2]); 
		translate ([l, -1, et]) rotate (a = [45, 0, 90]) cube (size = [w+2, h*2, h*2]); 
		if (notch==true){
			translate([2,w/2,h+0.001]) thumbNotch(10/2,72,t);
		}
        translate([sensor_pos_lid_x,wallThickness+1,-lidThickness]) cube([10,1,lidThickness*3]);
        translate([sensor_pos_lid_x,wallThickness+3,-lidThickness]) cube([10,1,lidThickness*3]);
        translate([sensor_pos_lid_x,wallThickness+5,-lidThickness]) cube([10,1,lidThickness*3]);
        translate([sensor_pos_lid_x,wallThickness+7,-lidThickness]) cube([10,1,lidThickness*3]);
	}

    

}

module thumbNotch(
	thumbR=12/2,
	angle=72,
	notchHeight=2){

	size=10*thumbR;

	rotate([0,0,90])
	difference(){
		translate([0,
					(thumbR*sin(angle)-notchHeight)/tan(angle),
					 thumbR*sin(angle)-notchHeight])
		rotate([angle,0,0])
		cylinder(r=thumbR,h=size,$fn=30);

		translate([-size,-size,0])
		cube(size*2);
	}
}

module round_cube(l=40,w=30,h=20,r=5,$fn=30){
	hull(){ 
		translate ([r, r, 0]) cylinder (h = h, r=r);
		translate ([r, w-r, 0]) cylinder (h = h, r=r);
		translate ([l-r,w-r, 0]) cylinder (h = h, r=r);
		translate ([l-r, r, 0]) cylinder (h = h, r=r);
	}
}
