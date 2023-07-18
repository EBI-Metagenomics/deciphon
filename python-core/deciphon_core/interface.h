struct dcp_press;
struct dcp_scan;
struct dcp_seq;
struct h3client_result;

// Scan parameters
struct dcp_scan_params
{
  int num_threads;
  double lrt_threshold;
  bool multi_hits;
  bool hmmer3_compat;
};

// Seq
typedef bool dcp_seq_next_fn(struct dcp_seq *, void *);
int dcp_seq_setup(struct dcp_seq *, long id, char const *name,
                  char const *data);
void dcp_seq_cleanup(struct dcp_seq *);

// Press
struct dcp_press *dcp_press_new(void);
int dcp_press_setup(struct dcp_press *, int gencode_id, float epsilon);
int dcp_press_open(struct dcp_press *, char const *hmm, char const *db);
long dcp_press_nproteins(struct dcp_press const *);
int dcp_press_next(struct dcp_press *);
bool dcp_press_end(struct dcp_press const *);
int dcp_press_close(struct dcp_press *);
void dcp_press_del(struct dcp_press const *);

// Scan
struct dcp_scan *dcp_scan_new(void);
int dcp_scan_dial(struct dcp_scan *, int port);
int dcp_scan_setup(struct dcp_scan *, struct dcp_scan_params);
void dcp_scan_del(struct dcp_scan const *);
int dcp_scan_run(struct dcp_scan *, char const *dbfile, dcp_seq_next_fn *callb,
                 void *userdata, char const *product_dir);

// Strerror
char const *dcp_strerror(int err);

// H3client
struct h3client_result *h3client_result_new(void);
void h3client_result_del(struct h3client_result const *);
int h3client_result_unpack(struct h3client_result *, FILE *);
int h3client_result_errnum(struct h3client_result const *);
char const *h3client_result_errstr(struct h3client_result const *);
void h3client_result_print_targets(struct h3client_result const *, FILE *);
void h3client_result_print_domains(struct h3client_result const *, FILE *);
void h3client_result_print_targets_table(struct h3client_result const *,
                                         FILE *);
void h3client_result_print_domains_table(struct h3client_result const *,
                                         FILE *);

FILE *fopen(char const *filename, char const *mode);
FILE *fdopen(int, char const *);
int fclose(FILE *);

extern "Python" bool next_seq_callb(struct dcp_seq *, void *);
