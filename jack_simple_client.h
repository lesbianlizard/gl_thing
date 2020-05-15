#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>


/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int
callback_jack_process (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in, *out;
  jack_sample_t *shared_buf_out;
	
	in = jack_port_get_buffer (jack_input_port, nframes);
	out = jack_port_get_buffer (jack_output_port, nframes);

  printf("[%s] copying raw data starting with %f\n", __func__, *in);
  
  // Copy audio to output port
	memcpy (
    out,
    in,
		sizeof (jack_default_audio_sample_t) * nframes);

  printf("[%s] About to copy memory to jack_raw_buf_pos == %li into jack_raw_buf == %p\n", __func__, jack_raw_buf_pos, jack_raw_buffer);

  if (jack_raw_buf_pos == 157)
  {
    printf("hey gdb, it's about to crash!\n");
  }
  
  shared_buf_out = jack_raw_buffer;

  memcpy(
    shared_buf_out,
    in,
    sizeof(jack_sample_t) * nframes);
    

	return 0;      
}

void
check_that_we_actually_wrote_jack_data(jack_sample_t* ptr)
{
  jack_sample_t sample;
  size_t idx, i;
  
  for (i = 0; i < jack_buffer_size; i++)
  {
    idx = jack_raw_buf_pos - 1 + i;
    sample = ptr[idx];
    printf("[%s] at index %li, jack_raw_buffer == %f\n", __func__, idx, sample);
  }
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
	exit (1);
}

void*
jack_main (void* mymutex)
{
	const char **ports;
	const char *client_name = "simple";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;
	
	/* open a client connection to the JACK server */
  //pthread_mutex_lock(&mymutex);

	jack_client = jack_client_open (client_name, options, &status, server_name);
	if (jack_client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(jack_client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (jack_client, callback_jack_process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (jack_client, jack_shutdown, 0);

	/* display the current sample rate. 
	 */

  // FIXME: we have to make a callback to handle this or the buffer size changing
  jack_sample_rate = jack_get_sample_rate(jack_client);
	printf ("engine sample rate: %" PRIu32 "\n",
		jack_sample_rate);

  jack_buffer_size = jack_get_buffer_size(jack_client);
	printf ("engine buffer size: %" PRIu32 "\n",
		jack_buffer_size);

	/* create two ports */

	jack_input_port = jack_port_register (jack_client, "input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);
	jack_output_port = jack_port_register (jack_client, "output",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);

	if ((jack_input_port == NULL) || (jack_output_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}

	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if (jack_activate (jack_client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	ports = jack_get_ports (jack_client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (1);
	}

	if (jack_connect (jack_client, ports[0], jack_port_name (jack_input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);
	
	ports = jack_get_ports (jack_client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}

	if (jack_connect (jack_client, jack_port_name (jack_output_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

	free (ports);

	/* keep running until stopped by the user */

	sleep (-1);

	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/

	jack_client_close (jack_client);
	exit (0);
}
