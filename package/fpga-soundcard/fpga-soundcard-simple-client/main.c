#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>


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

int write_sample_coefficients_from_file(snd_ctl_t *handle, snd_ctl_elem_id_t *id,  snd_ctl_elem_value_t *value, const char* file_name) {
	int err;
	char* data = (char*)malloc(8*sizeof(char));
	char read_byte;
	int bytes_read;
	FILE *fp;
	uint32_t bram_addr = 0x00000000;

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);
	snd_ctl_elem_id_set_name(id, "FPGA FIR Coefficients Values");
	if ((err = lookup_id(id, handle)) < 0){
	    return err;
	}

	if ((fp = fopen(file_name,"r")) == NULL) {
            fprintf(stderr, "Couldn't open the file you are requesting");
	    return -1;
	}

	snd_ctl_elem_value_set_id(value, id);

	// Verify by getting the ID from the value structure
        fprintf(stderr, "Control element name set to: %s\n", snd_ctl_elem_id_get_name(id));
        fprintf(stderr, "Control element numid set to: %u\n", snd_ctl_elem_id_get_numid(id));

        fprintf(stderr, "Reading file coefficients file, this can take a while...\n");
	while(1) {
	  // Adding BRAM destination address for our read bytes
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
	  snd_ctl_elem_set_bytes(value, data, sizeof(data));
	  if ((err = snd_ctl_elem_write(handle, value)) < 0) {
	    fprintf(stderr, "Control element write error: %s\n",
		    snd_strerror(err));
	    return err;
	  }

	  bram_addr++;
	}

	fclose(fp);
	return 0;
}

int main(int argc, char *argv[]){
	int err;
	snd_ctl_t *handle;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *value;
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&value);

	if ((err = snd_ctl_open(&handle, "hw:0", 0)) < 0) {
		fprintf(stderr, "Card open error: %s\n", snd_strerror(err));
		return err;
	}

	if (write_sample_coefficients_from_file(handle, id, value, "./filter_file.bin")!=0) {
	  fprintf(stderr, "The program terminated unexpectedly");
	  return -1;
	}

	// Reading test section:
	/* unsigned char data[8] = { 0x00, 0x00, 0x00, 0x00, 0x0A, 0x0B, 0x0C, 0x0D}; */
        /* snd_ctl_elem_set_bytes(value, data, sizeof(data)); */

	/* if ((err = snd_ctl_elem_read(handle, value)) < 0) { */
	/*   fprintf(stderr, "Control element read error: %s\n", */
	/*   snd_strerror(err)); */
	/*   return err; */
	/* } */
	
        /* unsigned char* reading_result = malloc(512*sizeof(unsigned char)); */

	/* memcpy(reading_result, snd_ctl_elem_value_get_bytes(value), 512); */
	/* printf("%c", *(reading_result+0)); */

	// Free allocated memory
	snd_ctl_elem_id_clear(id);
	snd_ctl_elem_value_clear(value);

	// Close the control interface
	snd_ctl_close(handle);

	return 0;
}
