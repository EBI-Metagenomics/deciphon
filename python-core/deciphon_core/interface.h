struct press;
struct scan;
struct h3client_result;

// parameters
struct params
{
  int  num_threads;
  bool multi_hits;
  bool hmmer3_compat;
};
int params_setup(struct params *, int num_threads, bool multi_hits, bool hmmer3_compat);

// Press
struct press *press_new(void);
int           press_setup(struct press *, int gencode_id, float epsilon);
int           press_open(struct press *, char const *hmm, char const *db);
long          press_nproteins(struct press const *);
int           press_next(struct press *);
bool          press_end(struct press const *);
int           scan_progress(struct scan const *);
int           press_close(struct press *);
void          press_del(struct press const *);

// Scan
struct scan *scan_new(struct params);
void         scan_del(struct scan const *);
int          scan_dial(struct scan *, int port);
int          scan_open(struct scan *, char const *dbfile);
int          scan_close(struct scan *);
int          scan_add(struct scan *, long id, char const *name, char const *data);
int          scan_run(struct scan *, char const *product_dir);


// Strerror
char const *string_error(int err);

// H3client
struct h3client_result *h3client_result_new(void);
void                    h3client_result_del(struct h3client_result const *);
int                     h3client_result_unpack(struct h3client_result *, FILE *);
int                     h3client_result_errnum(struct h3client_result const *);
char const *            h3client_result_errstr(struct h3client_result const *);
void                    h3client_result_print_targets(struct h3client_result const *, FILE *);
void                    h3client_result_print_domains(struct h3client_result const *, FILE *);
void                    h3client_result_print_targets_table(struct h3client_result const *, FILE *);
void                    h3client_result_print_domains_table(struct h3client_result const *, FILE *);

// Stdio
FILE *fopen(char const *filename, char const *mode);
FILE *fdopen(int, char const *);
int   fclose(FILE *);
