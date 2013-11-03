/*
C 
*/
#include <stdio.h>
#define pi 3.1415926

double circle_length(double r){
    return 2 * pi * r;
}

double circle_area(double r){
    return pi * r * r;
}

double sphere_area(double r){
    return 4 * pi * r * r;
}

double sphere_volume(double r){
    return 0.75 * pi * r * r *r;
}

double circle_column_volume(double r, double h){
    return circle_area(r) * h;
}

int main(){
    double length,area,volume,r,h;
    printf("input circle radius\n");
    scanf("%lf", &r);
    length = circle_length(r);
    printf("length: %.2f\n",length);
    area = circle_area(r);
    printf("circle_area: %.2f\n",area);
    area = sphere_area(r);
    printf("sphere_area: %.2f\n", area);
    volume = sphere_volume(r);
    printf("sphere_volume: %.2f\n", volume);
    printf("input height\n");
    scanf("%lf", &h);
    volume = circle_column_volume(r,h);
    printf("circle_column_volume: %.2f\n",volume);
}