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
	snd_ctl_elem_id_set_name(id, "FPGA FIR Coefficients");
	if ((err = lookup_id(id, handle)) < 0){
	    return err;
	  }

	snd_ctl_elem_value_set_id(value, id);
	snd_ctl_elem_value_set_integer(value, 0, 55);
	snd_ctl_elem_value_set_integer(value, 1, 77);

	if ((err = snd_ctl_elem_write(handle, value)) < 0) {
	  fprintf(stderr, "Control element write error: %s\n",
	  snd_strerror(err));
	  return err;
	}

	snd_ctl_elem_id_clear(id);
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_CARD);
	snd_ctl_elem_id_set_name(id, "Headphone Playback Switch");
	if (err = lookup_id(id, handle))
	  return err;

	snd_ctl_elem_value_clear(value);
	snd_ctl_elem_value_set_id(value, id);
	snd_ctl_elem_value_set_boolean(value, 1, 1);

        if ((err = snd_ctl_elem_write(handle, value)) < 0) {
          fprintf(stderr, "Control element write error: %s\n",
                  snd_strerror(err));
          return err;
        }

	return 0;
}
