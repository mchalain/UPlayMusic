#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <dlna.h>

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
        dlna_org_flags_t flags;
        dlna_capability_mode_t cap;

        flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
                DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
                DLNA_ORG_FLAG_CONNECTION_STALL |
                DLNA_ORG_FLAG_DLNA_V15;
        cap = DLNA_CAPABILITY_DLNA;

        /* init DLNA stack */
        dlna = dlna_init ();
        dlna_set_org_flags (dlna, flags);
        dlna_set_verbosity (dlna, DLNA_MSG_INFO);
        dlna_set_capability_mode (dlna, cap);
        dlna_set_extension_check (dlna, 1);

        /* init Media profiler */
        profiler = &mpg123_profiler;
        mpg123_profiler_init ();
        dlna_add_profiler (dlna, profiler);

        /* define NIC to be used */
        dlna_set_interface (dlna, ip_address);
        dlna_set_port (dlna, listen_port);

        /* set some UPnP device properties */
        device = dlna_device_new ();
        dlna_device_set_type (device, DLNA_DEVICE_TYPE_DMR,"DMR");
        dlna_device_set_friendly_name (device, name);
        dlna_device_set_uuid (device, uuid);
        dlna_device_set_serial_number (device, serial_number);

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
