#include <stdio.h>
#include <strings.h>
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

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);
	snd_ctl_elem_id_set_name(id, "FPGA FIR Coefficients Values");
	if ((err = lookup_id(id, handle)) < 0){
	    return err;
	  }

	snd_ctl_elem_value_set_id(value, id);

	// Verify by getting the ID from the value structure
        fprintf(stderr, "Control element name set to: %s\n", snd_ctl_elem_id_get_name(id));
        fprintf(stderr, "Control element numid set to: %u\n", snd_ctl_elem_id_get_numid(id));
	
	unsigned char data[8] = { 0x00, 0x00, 0x00, 0x00, 0x0A, 0x0B, 0x0C, 0x0D};
        snd_ctl_elem_set_bytes(value, data, sizeof(data));

	// Writing test section:
	if ((err = snd_ctl_elem_write(handle, value)) < 0) {
	  fprintf(stderr, "Control element write error: %s\n",
	  snd_strerror(err));
	  return err;
	}

	// Reading test section:
	if ((err = snd_ctl_elem_read(handle, value)) < 0) {
	  fprintf(stderr, "Control element read error: %s\n",
	  snd_strerror(err));
	  return err;
	}
	
        unsigned char* reading_result = malloc(512*sizeof(unsigned char));

	memcpy(reading_result, snd_ctl_elem_value_get_bytes(value), 512);
	printf("%c", *(reading_result+0));

	// Free allocated memory
	snd_ctl_elem_id_clear(id);
	snd_ctl_elem_value_clear(value);
	free(reading_result);

	// Close the control interface
	snd_ctl_close(handle);

	return 0;
}
