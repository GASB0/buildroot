#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "error.h"
#include <sys/types.h>
#include <strings.h>
#include <getopt.h>

#define PROGRAM_NAME "simple_filter_client"


int lookup_id(snd_ctl_elem_id_t *id, snd_ctl_t *handle)
{
	int err;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_info_alloca(&info);

	snd_ctl_elem_info_set_id(info, id);
	if ((err = snd_ctl_elem_info(handle, info)) < 0) {
		fprintf(stderr, "Cannot find the given element from card\n");
		return err;
	}
	snd_ctl_elem_info_get_id(info, id);

	return 0;
}

int set_max_addr(snd_ctl_t *handle, snd_ctl_elem_id_t *id, snd_ctl_elem_value_t *value, int ncoefficients) {
  int err;
  int data = ncoefficients & ~(0xFFFF<<16);
  snd_ctl_elem_info_t *info;  
  snd_ctl_elem_info_alloca(&info);

  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);
  snd_ctl_elem_id_set_name(id, "FPGA FIR Coefficients Number");

  if ((err = lookup_id(id, handle)) < 0){
    return err;
  }

  snd_ctl_elem_value_set_id(value, id);

  // Writing bytes to codec control
  snd_ctl_elem_set_bytes(value, &data, 2);
  if ((err = snd_ctl_elem_write(handle, value)) < 0) {
    fprintf(stderr, "Control element write error: %s\n",
	    snd_strerror(err));
    return err;
  }

  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  return 0;
}

int get_max_addr(snd_ctl_t *handle, snd_ctl_elem_id_t *id,  snd_ctl_elem_value_t *value){
  int err;
  int maxAddr;
  snd_ctl_elem_info_t *info;  
  snd_ctl_elem_info_alloca(&info);

  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);
  snd_ctl_elem_id_set_name(id, "FPGA FIR Coefficients Number");

  if ((err = lookup_id(id, handle)) < 0){
    return err;
  }

  snd_ctl_elem_info_alloca(&info);
  snd_ctl_elem_info_set_id(info, id);

  if ((err = snd_ctl_elem_info(handle, info)) < 0) {
    fprintf(stderr, "Cannot get element info: %s\n", snd_strerror(err));
    return -1;
  }
  
  maxAddr = snd_ctl_elem_info_get_max(info);

  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  return maxAddr;
}

int write_sample_coefficients_from_file(snd_ctl_t *handle, snd_ctl_elem_id_t *id,  snd_ctl_elem_value_t *value, const char* file_name, int ncoefficients) {
	int err;
	char* data = (char*)malloc(8*sizeof(char));
	char read_byte;
	int bytes_read;
	long int file_size = 0;
	FILE *fp = NULL;
	uint32_t bram_addr = 0x00000000;
	int maxAddr=get_max_addr(handle, id, value);

	// Opening the file
	if ((fp = fopen(file_name,"r")) == NULL) {
            fprintf(stderr, "Couldn't open the file you are requesting");
	    return -1;
	}

	// Getting the size of the file in bytes
	fseek(fp, 0, SEEK_END);
	file_size=ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Setting the control to manipulate coefficient values
	snd_ctl_elem_id_clear(id);
	snd_ctl_elem_value_clear(value);
	snd_ctl_elem_id_set_name(id, "FPGA FIR Coefficients Values");
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);

	if ((err = lookup_id(id, handle)) < 0){
	    return err;
	}

	snd_ctl_elem_value_set_id(value, id);
        fprintf(stderr, "Control element name set to: %s\n", snd_ctl_elem_id_get_name(id));
        fprintf(stderr, "Control element numid set to: %u\n", snd_ctl_elem_id_get_numid(id));
	
	// Checking that the number of coefficients can by set into the FPGA
	if(ncoefficients > maxAddr){
	  fprintf(stderr, "You are trying to write more coefficients into the FPGA than it has configured\n");
	  return -1;
	}

	if(ncoefficients > file_size/4){
	  fprintf(stderr, "You are trying to write more coefficients into the FPGA than available in your file\n");
	  return -1;
	}

	// Performing the writing of the coefficients from file to FPGA
        fprintf(stderr, "Reading coefficients file and writing into FPGA, this can take a while...\n");
	while(1) {
	  data[0] = (bram_addr >> 0)  & 0xFF; // LSB
	  data[1] = (bram_addr >> 8)  & 0xFF;
	  data[2] = (bram_addr >> 16) & 0xFF;
	  data[3] = (bram_addr >> 24) & 0xFF; // MSB

	  // Actually reading the bytes from the file
	  bytes_read=fread(&data[4], sizeof(char), 4, fp);
	  if(bytes_read < 4) {
	    if(feof(fp)) {
	      if (bytes_read > 0) {
		fprintf(stderr, "Something's wrong with your file, check your format!\n");
		return -1;
	      }
	      fprintf(stderr, "Whole file succesffully read\n");
	      break;
	    } else {
	      perror("Error reading from file");
	      return -1;
	    }
	  }

	  // Writing bytes to codec control
	  snd_ctl_elem_set_bytes(value, data, 8*sizeof(char));
	  if ((err = snd_ctl_elem_write(handle, value)) < 0) {
	    fprintf(stderr, "Control element write error: %s\n",
		    snd_strerror(err));
	    return err;
	  }

	  if ((int)bram_addr == ncoefficients)
	    break;

	  bram_addr++;
	}

	fclose(fp);
	snd_ctl_elem_id_clear(id);
	snd_ctl_elem_value_clear(value);
	free(data);

	return 0;
}

int read_sample_coefficients_to_file(snd_ctl_t *handle, snd_ctl_elem_id_t *id,  snd_ctl_elem_value_t *value, const char* file_name, int ncoefficients) {
  int err;
  unsigned char* data = malloc(8*sizeof(unsigned char));
  unsigned char* reading_result = malloc(4*sizeof(unsigned char));
  uint32_t bram_addr = 0x00000000;
  size_t bytes_written;
  int maxAddr=get_max_addr(handle, id, value);
  FILE *fp;

  // Opening the file
  if ((fp=fopen(file_name, "w")) == NULL){
    fprintf(stderr, "Couldn't open the file you are requesting");
    return -1;
  }

  // Checking the number of coefficients you can actually write
  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);
  snd_ctl_elem_id_set_name(id, "FPGA FIR Coefficients Values");

  if ((err = lookup_id(id, handle)) < 0){
    return err;
  }

  // Verify by getting the ID from the value structure
  snd_ctl_elem_value_set_id(value, id);
  fprintf(stderr, "Control element name set to: %s\n", snd_ctl_elem_id_get_name(id));
  fprintf(stderr, "Control element numid set to: %u\n", snd_ctl_elem_id_get_numid(id));

  // Checking that the number of coefficients can by set into the FPGA
  if(ncoefficients > maxAddr){
    fprintf(stderr, "You are trying to write more coefficients into the FPGA than it has configured\n");
    return -1;
  }

  fprintf(stderr, "Writing coefficients from FPGA to file, this can take a while...\n");
  while(1){
    data[0] = (bram_addr >> 0)  & 0xFF; // LSB
    data[1] = (bram_addr >> 8)  & 0xFF;
    data[2] = (bram_addr >> 16) & 0xFF;
    data[3] = (bram_addr >> 24) & 0xFF; // MSB

    snd_ctl_elem_set_bytes(value, data, sizeof(data));

    if ((err = snd_ctl_elem_read(handle, value)) < 0) {
      fprintf(stderr, "Control element read error: %s\n",
	      snd_strerror(err));
      return err;
    }
	
    memcpy(reading_result, ((unsigned char*)snd_ctl_elem_value_get_bytes(value))+4, 4*sizeof(unsigned char));

    bytes_written=fwrite(reading_result, sizeof(char), 4, fp);
    if(bytes_written!=4){
      return -1;
    }

    if ((int)bram_addr == ncoefficients)
      break;

    bram_addr++;
  }

  fclose(fp);
  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  free(data);

  return 0;
}

int check_controls(snd_ctl_t *handle, snd_ctl_elem_id_t *id,  snd_ctl_elem_value_t *value)
{
  int err;
  unsigned char* reading_result = malloc(4*sizeof(unsigned char));

  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);
  snd_ctl_elem_id_set_name(id, "FPGA FIR General Controls");
  if ((err = lookup_id(id, handle)) < 0){
    return err;
  }

  snd_ctl_elem_value_set_id(value, id);

  // Verify by getting the ID from the value structure
  /* fprintf(stderr, "Control element name set to: %s\n", snd_ctl_elem_id_get_name(id)); */
  /* fprintf(stderr, "Control element numid set to: %u\n", snd_ctl_elem_id_get_numid(id)); */

  if ((err = snd_ctl_elem_read(handle, value)) < 0) {
    fprintf(stderr, "Control element read error: %s\n",
	    snd_strerror(err));
    return err;
  }

  memcpy(reading_result, (unsigned char*)snd_ctl_elem_value_get_bytes(value), 4*sizeof(unsigned char));

  fprintf(stderr, "State of the filter (on=1, off=0): %d\n", (int)(*reading_result & 0x01));
  fprintf(stderr, "Max address you can access: 0x%04x\n", get_max_addr(handle, id, value));

  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);
  return 0;
}

void print_help() {
    printf("Usage: %s [-h] [-n coeffNum -rw filename] \n", PROGRAM_NAME);
    printf("  -h           Display this help message\n");
    printf("  -i           Check general controls of the codec\n");
    printf("  -n           Set the number of coefficients to either read or write from the codec\n");
    printf("  -s           Set max possible accessible address in the FPGA (use the prefix 0x to use hex numbers)\n");
    printf("  -r           Reads n filter coefficients from codec to $filename\n");
    printf("  -w           Writes n filter coefficients to the codec from $filename\n");
}

int main(int argc, char *argv[]){
  int err, opt;
  int ncoefficients = -1;
  char *filename = NULL;
  char *hex_prefix = "0x";

  snd_ctl_t *handle;
  snd_ctl_elem_id_t *id;
  snd_ctl_elem_value_t *value;

  snd_ctl_elem_id_alloca(&id);
  snd_ctl_elem_value_alloca(&value);

  if ((err = snd_ctl_open(&handle, "hw:0", 0)) < 0) {
    fprintf(stderr, "Card open error: %s\n", snd_strerror(err));
    return err;
  }

  while ((opt=getopt(argc, argv, "his:n:r:w:")) != -1){
    switch(opt){
    case 'h':
      print_help();
      break;
    case 'i':
      check_controls(handle, id, value);
      break;
    case 'n':
      ncoefficients = atoi(optarg);
      break;
    case 's':
      if (strncmp(optarg, "0x",2) == 0){
	set_max_addr(handle, id, value, strtol(optarg+2, NULL, 16));
      } else {
	set_max_addr(handle, id, value, atoi(optarg));
      }
      break;
    case 'r':
      read_sample_coefficients_to_file(handle, id, value, optarg, ncoefficients);
      break;
    case 'w':
      write_sample_coefficients_from_file(handle, id, value, optarg, ncoefficients);
      break;
    default:
      print_help();
      exit(EXIT_FAILURE);
    }
  }
  
  // Free allocated memory
  snd_ctl_elem_id_clear(id);
  snd_ctl_elem_value_clear(value);

  // Close the control interface
  snd_ctl_close(handle);

  return 0;
}
