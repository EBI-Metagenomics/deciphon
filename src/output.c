#include "deciphon/deciphon.h"
#include "lib/list.h"
#include "metadata.h"
#include "nmm/nmm.h"
#include "profile.h"
#include "util.h"
#include <string.h>

struct offset
{
    uint64_t    value;
    struct list link;
};

struct metadata
{
    struct dcp_metadata const* mt;
    struct list                link;
};

struct dcp_output
{
    char const*        filepath;
    FILE*              stream;
    uint32_t           nprofiles;
    struct list        profile_offsets;
    struct list        profile_metadatas;
    uint32_t           profile_metadata_size;
    char const*        tmp_filepath;
    struct nmm_output* tmp_output;
    bool               closed;
};

static char const* create_tmp_filepath(char const* filepath);

int dcp_output_close(struct dcp_output* output)
{
    if (output->closed)
        return 0;

    int   errno = 0;
    FILE* tmp_stream = NULL;
    if (nmm_output_close(output->tmp_output)) {
        errno = 1;
        goto cleanup;
    }
    tmp_stream = fopen(output->tmp_filepath, "rb");
    if (!tmp_stream) {
        error("failed to open %s", output->tmp_filepath);
        errno = 1;
        goto cleanup;
    }

    if (fwrite(&output->nprofiles, sizeof(output->nprofiles), 1, output->stream) < 1) {
        error("could not write nprofiles");
        errno = 1;
        goto cleanup;
    }

    struct offset* offset = NULL;
    uint64_t       start = sizeof(output->nprofiles) + output->nprofiles * sizeof(offset->value);
    start += output->profile_metadata_size;
    for (struct list* i = list_head(&output->profile_offsets); i; i = list_next(&output->profile_offsets, i)) {
        offset = container_of(i, struct offset, link);
        uint64_t v = start + offset->value;
        if (fwrite(&v, sizeof(v), 1, output->stream) < 1) {
            error("could not write offset");
            errno = 1;
            goto cleanup;
        }
    }

    for (struct list* i = list_head(&output->profile_metadatas); i; i = list_next(&output->profile_metadatas, i)) {
        struct metadata* mt = container_of(i, struct metadata, link);
        if (profile_metadata_write(mt->mt, output->stream)) {
            errno = 1;
            goto cleanup;
        }
    }

    if (fcopy_content(output->stream, tmp_stream)) {
        error("failed to copy file");
        errno = 1;
        goto cleanup;
    }

cleanup:
    if (output->stream && fclose(output->stream))
        error("failed to close file %s", output->filepath);

    if (tmp_stream && fclose(tmp_stream))
        error("failed to close file %s", output->tmp_filepath);

    output->closed = true;
    return errno;
}

struct dcp_output* dcp_output_create(char const* filepath)
{
    struct dcp_output* output = malloc(sizeof(*output));
    output->filepath = strdup(filepath);
    output->stream = NULL;
    output->nprofiles = 0;
    list_init(&output->profile_offsets);
    list_init(&output->profile_metadatas);
    output->profile_metadata_size = 0;
    output->tmp_filepath = create_tmp_filepath(filepath);
    output->tmp_output = NULL;
    output->closed = false;

    if (!(output->stream = fopen(filepath, "wb"))) {
        error("could not open file %s for writing", filepath);
        goto err;
    }

    if (!(output->tmp_output = nmm_output_create(output->tmp_filepath))) {
        error("could not create output");
        goto err;
    }

    return output;

err:
    if (output->filepath)
        free((void*)output->filepath);

    if (output->stream)
        fclose(output->stream);

    if (output->tmp_filepath)
        free((void*)output->tmp_filepath);

    if (output->tmp_output)
        nmm_output_destroy(output->tmp_output);

    free(output);
    return NULL;
}

int dcp_output_destroy(struct dcp_output* output)
{
    int errno = 0;
    if (nmm_output_destroy(output->tmp_output)) {
        error("could not destroy temporary output");
        errno = 1;
        goto cleanup;
    }

    if (dcp_output_close(output)) {
        errno = 1;
        goto cleanup;
    }
cleanup:
    if (output->filepath)
        free((void*)output->filepath);

    if (output->tmp_filepath)
        free((void*)output->tmp_filepath);

    struct list* i = list_head(&output->profile_offsets);
    while (i) {
        struct list tmp = *i;
        list_del(i);
        free(container_of(i, struct offset, link));
        i = list_next(&output->profile_offsets, &tmp);
    }

    i = list_head(&output->profile_metadatas);
    while (i) {
        struct list tmp = *i;
        list_del(i);
        struct metadata* mt = container_of(i, struct metadata, link);
        dcp_metadata_destroy(mt->mt);
        free(mt);
        i = list_next(&output->profile_metadatas, &tmp);
    }

    free(output);
    return errno;
}

int dcp_output_write(struct dcp_output* output, struct dcp_profile const* prof)
{
    int64_t v = nmm_output_ftell(output->tmp_output);
    if (v < 0) {
        error("failed to ftell");
        return 1;
    }
    struct offset* offset = malloc(sizeof(*offset));
    offset->value = (uint64_t)v;
    list_init(&offset->link);
    list_add(&output->profile_offsets, &offset->link);

    struct metadata* mt = malloc(sizeof(*mt));
    mt->mt = profile_metadata_clone(dcp_profile_metadata(prof));
    list_init(&mt->link);
    list_add(&output->profile_metadatas, &mt->link);

    output->nprofiles++;
    output->profile_metadata_size += profile_metadata_size(mt->mt);

    return nmm_output_write(output->tmp_output, dcp_profile_nmm_profile(prof));
}

static char const* create_tmp_filepath(char const* filepath)
{
    char* tmp_filepath = malloc(sizeof(char) * (strlen(filepath) + 5));
    memcpy(tmp_filepath, filepath, sizeof(char) * strlen(filepath));
    tmp_filepath[strlen(filepath) + 0] = '.';
    tmp_filepath[strlen(filepath) + 1] = 't';
    tmp_filepath[strlen(filepath) + 2] = 'm';
    tmp_filepath[strlen(filepath) + 3] = 'p';
    tmp_filepath[strlen(filepath) + 4] = '\0';
    return tmp_filepath;
}
