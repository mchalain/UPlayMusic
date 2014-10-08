#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <upnp/dlna.h>

extern const dlna_profiler_t mpg123_profiler;
extern int mpg123_profiler_init ();

struct upnp {
        dlna_t *dlna;
};

struct upnp * upnp_start(char *name, char *uuid, char *serial_number, char *ip_address, int listen_port)
{
	struct upnp *upnp;
	upnp = malloc(sizeof(*upnp));
	memset(upnp, 0, sizeof(*upnp));

        dlna_t *dlna;
        dlna_device_t *device;
        const dlna_profiler_t *profiler;
        dlna_capability_mode_t cap;

        cap = DLNA_CAPABILITY_DLNA;

        /* init DLNA stack */
        dlna = dlna_init ();
        dlna_set_verbosity (dlna, DLNA_MSG_INFO);
        dlna_set_extension_check (dlna, 1);

        /* init Media profiler */
        profiler = &mpg123_profiler;
        mpg123_profiler_init ();
        dlna_add_profiler (dlna, profiler);

        /* define NIC to be used */
        if (!ip_address)
          ip_address = strdup ("eth0");
        dlna_set_interface (dlna, ip_address);
        dlna_set_port (dlna, listen_port);

        /* set some UPnP device properties */
        device = dlna_device_new (cap);
        dlna_device_set_type (device, DLNA_DEVICE_TYPE_DMR,"DMR");
        dlna_device_set_friendly_name (device, name);
        dlna_device_set_uuid (device, uuid);
        dlna_device_set_serial_number (device, serial_number);
        dlna_device_set_model_description (device, "UPlayMusic : DLNA Media Renderer");
        dlna_device_set_model_name (device, name);
        dlna_device_set_model_number (device, "001");
        dlna_device_set_manufacturer (device, "Marc Chalain");
        dlna_device_set_manufacturer_url (device, "http://github.com/mchalain/UPlayMusic");

        dlna_service_register (device, cms_service_new(dlna));
        dlna_service_register (device, rcs_service_new(dlna));
        dlna_service_register (device, avts_service_new(dlna));

        dlna_set_device (dlna, device);

        if (dlna_start (dlna) != DLNA_ST_OK)
        {
                printf ("DMR init went wrong\n");
                dlna_uninit (dlna);
                return NULL;
        }
        upnp->dlna = dlna;
	return upnp;
exit_error:
	free(upnp);
	return NULL;
}

void upnp_stop(struct upnp *upnp)
{
	if (upnp)
	{
                /* DMS shutdown */
                dlna_stop (upnp->dlna);

                /* DLNA stack cleanup */
                dlna_uninit (upnp->dlna);
	}
}
