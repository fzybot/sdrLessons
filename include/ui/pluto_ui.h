
extern sdr_global_t *sdr;

void show_iq_scatter_plot(sdr_global_t *sdr, std::vector<std::complex<float>> &samples);

void show_main_window(sdr_global_t *sdr);
void show_properties_window(sdr_global_t *sdr);

void run_gui(sdr_global_t *sdr);
