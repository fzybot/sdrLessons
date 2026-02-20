
extern sdr_global_t *sdr;

void show_iq_scatter_plot(sdr_global_t *sdr, std::vector<std::complex<double>> &samples);
void test_pulse_shaping_srrc(sdr_global_t *sdr);
void test_pulse_shaping_hamming(sdr_global_t *sdr);

void show_main_window(sdr_global_t *sdr);
void show_properties_window(sdr_global_t *sdr);
void show_global_menu_bar(sdr_global_t *sdr);
void show_test_sdr_set(sdr_global_t *sdr);

void run_gui(sdr_global_t *sdr);
