#include "output_module.h"
#include <alsa/asoundlib.h>

int output_alsa_get_volume(float *v) {
	return 0;
}
int output_alsa_set_volume(float value) {
	return 0;
}
int output_alsa_get_mute(int *m) {
	return 0;
}
int output_alsa_set_mute(int m) {
	return 0;
}


snd_ctl_t *
output_alsa_init(int cardNum, char *cardName)
{
	snd_ctl_t *cardHandle;
	while (cardNum < 0 && cardName != NULL)
	{
		int err;
		snd_ctl_card_info_t *cardInfo;
		if ((err = snd_card_next(&cardNum)) < 0)
		{
			return -err;
		}
		sprintf(str, "hw:%i", cardNum);
		if ((err = snd_ctl_open(&cardHandle, str, 0)) < 0)
		{
			printf("Can't open card %i: %s\n", cardNum, snd_strerror(err));
			continue;
		}
	}
	snd_mixer_t *mixerHandle;
	int mode = 0;
	if ((err = snd_mixer_open(&mixerHandle, mode)) < 0
	{
		return -err;
	}
	snd_mixer_selem_id_t *sid;
	snd_mixer_selem_id_alloca(&sid);

	if ((err = snd_mixer_attach(mixerHandle, cardHandle)) < 0)
	{
		goto mixer_error;
	}
	if ((err = snd_mixer_selem_register(mixerHandle, NULL, NULL)) < 0) {
		goto mixer_error;
	}
	err = snd_mixer_load(mixerHandle);
	if (err < 0) {
		goto mixer_error;
	}
	snd_mixer_elem_t *elem;
	for (elem = snd_mixer_first_elem(mixerHandle); elem; elem = snd_mixer_elem_next(elem)) {
		snd_mixer_selem_get_id(elem, sid);
		if (!snd_mixer_selem_is_active(elem))
			continue;
		if (_output_alsa_checkElement(mixerHandle, sid) > 0)
			break;
	}

mixer_error:
	snd_mixer_close(handle);
	return -err;
}

int
_output_alsa_checkElement(snd_mixer_t *mixerHandle, snd_mixer_selem_id_t *sid)
{
	int ret = 0;
	snd_mixer_elem_t *elem;

	elem = snd_mixer_find_selem(mixerHandle, sid);
	if (snd_mixer_selem_has_common_volume(elem))
		ret &= 0x1;
	else if (snd_mixer_selem_has_playback_volume(elem))
		ret &= 0x1;
	if (snd_mixer_selem_has_common_switch(elem))
		ret &= 0x2;
	else if (snd_mixer_selem_has_playback_switch(elem))
		ret &= 0x2;
	return (ret & 0x3) == 0x3;
}

static snd_ctl_card_info_t *
output_alsa_cardInfo_create(snd_ctl_t *cardHandle)
{
	snd_ctl_card_info_t *cardInfo;

	snd_ctl_card_info_alloca(&cardInfo);
	if ((err = snd_ctl_card_info(cardHandle, cardInfo)) < 0)
		return NULL;
	return cardInfo;
}

