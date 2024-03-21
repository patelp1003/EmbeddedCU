/*
 * Userspace program that communicates with the vga_ball device driver
 * through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */

#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
int vga_ball_fd;

/* Read and print the background color */
void print_background_color() {
  vga_ball_arg_t vla;
  
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
      return;
  }
  printf("%02x %02x %02x\n",
	 vla.background.red, vla.background.green, vla.background.blue);
}

/* Set the background color */
void set_background_color(const vga_ball_color_t *c)
{
  vga_ball_arg_t vla;
  vla.background = *c;
      if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_SET_BACKGROUND) failed");
      return;
  }
}

void set_hv(const vga_ball_hv_t *c)
{
  vga_ball_arg_t vla;
  vla.hv = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_HV, &vla)) {
      perror("ioctl(VGA_BALL_SET_HV) failed");
      return;
  }
}


int main()
{
  vga_ball_arg_t vla;
  int i;
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t colors[] = {
    { 0xff, 0x00, 0x00 }, /* Red */
    { 0x00, 0xff, 0x00 }, /* Green */
    { 0x00, 0x00, 0xff }, /* Blue */
    { 0xff, 0xff, 0x00 }, /* Yellow */
    { 0x00, 0xff, 0xff }, /* Cyan */
    { 0xff, 0x00, 0xff }, /* Magenta */
    { 0x80, 0x80, 0x80 }, /* Gray */
    { 0x00, 0x00, 0x00 }, /* Black */
    { 0xff, 0xff, 0xff }  /* White */
  };

  vga_ball_hv_t hv_val = {0, 0};

# define COLORS 9

  printf("VGA ball Userspace program started\n");

  if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  printf("initial state: ");
  //print_background_color();
  unsigned int radius = 10;
  int h_count = radius;
  int v_count = radius;
  bool h_flag = false;
  bool v_flag = false;
  for (i = 0 ; i < 20000; i++) {
    set_background_color(&colors[2]);
    hv_val.h = h_count;
    hv_val.v = v_count;

    if(v_count < 480-radius &&v_flag == false){
	v_count++;
    }
 
    if(v_count >= 480-radius || v_flag == true){
	v_count--;
        v_flag = true;
       if(v_count == radius)
	v_flag = false;
    }

    if(h_count < 640-radius && h_flag == false){
        h_count++;
    }
    if(h_count >= 640-radius || h_flag == true){
        h_count--;
	h_flag = true;
	if(h_count == radius)
	 h_flag = false;
    }


    set_hv(&hv_val);
    print_background_color();
    usleep(9000);
    
  }


  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
