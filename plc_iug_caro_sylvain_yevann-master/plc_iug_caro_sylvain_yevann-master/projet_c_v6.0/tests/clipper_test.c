#include <stdio.h>
#include <stdint.h>
#include "hw_interface.h"

#include "clipper.h"

int main()
{

// Init acces to hardware.
        hw_init();

        ei_rect_t clipper;
        clipper.top_left.x = 0;
        clipper.top_left.y = 0;
        clipper.size.width = 100;
        clipper.size.height = 100;

        // line clipper test
//        ei_point_t p1;
//        p1.x = -25;
//        p1.y = 50;
//
//        ei_point_t p2;
//        p2.x = 50;
//        p2.y = 150;
//
//        ei_clipped_point_t cp1;
//        cp1.parent_point = p1;
//
//        ei_clipped_point_t cp2;
//        cp2.parent_point = p2;
//
//        uint8_t test = ei_line_clipper(&cp1, &cp2, &clipper);
//
//        printf("ligne toujours présente : %u \n", test);
//
//        printf("cp1.x: %u / cp1.y: %u \n", cp1.point.x, cp1.point.y);
//
//        printf("cp2.x: %u / cp2.y: %u \n", cp2.point.x, cp2.point.y);

        // clipper_SH test
        ei_linked_point_t p1, p2, p3;
        p1.point.x = -50;
        p1.point.y = -50;
        p1.next = &p2;
        p2.point.x = 100;
        p2.point.y = 0;
        p2.next = &p3;
        p3.point.x = 50;
        p3.point.y = 150;
        p3.next = &p1;

        ei_linked_clipped_point_t *poly = ei_polygone_clipper_SH(&p1, &clipper);

        printf("fini\n");

// Free hardware resources.
        hw_quit();

// Terminate program with no error code.
        return 0;
}
