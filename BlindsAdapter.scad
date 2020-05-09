

$fn=144;

shaft_diameter = 6.5;

difference() {
    cylinder(18, 7, 7);
    
    //Blinds shaft
    translate([-shaft_diameter/2, -shaft_diameter/2, 7])
        cube([shaft_diameter, shaft_diameter, 11]);
    
    // Motor shaft
    translate([-6/2, -3.5/2, 0])
        cube([6, 3.5, 7]);
};