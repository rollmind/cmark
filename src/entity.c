#include "utf8.h"
#include "chunk.h"
#include "buffer.h"
#include "cmark_ctype.h"
#include "entity.h"
#include "entities.inc"

#ifndef MIN
#define MIN(x, y) ((x < y) ? x : y)
#endif

int scan_entity(cmark_chunk *chunk, bufsize_t offset) {
  bufsize_t limit = MIN(offset + CMARK_ENTITY_MAX_LENGTH, chunk->len);
  bufsize_t i = offset;
  const unsigned char *data = chunk->data;
  bool numerical = false;
  bool hex = false;
  bool nonempty = false;

  if (data == NULL || offset > chunk->len) {
    return 0;
  }

  if (data[i] == '&') {
    i++;
  } else {
    return 0;
  }

  if (data[i] == '#') { // numerical
    i++;
    numerical = true;
    if (data[i] == 'X' || data[i] == 'x') {
      i++;
      hex = true;
    }
    limit = i + 7;
  } else {
    if (cmark_isalpha(data[i])) {
      i++;
      nonempty = true;
    } else {
      return 0;
    }
  }

  while (i <= limit && data[i]) {
    if (hex && !cmark_ishexdigit(data[i])) {
        break;
    } else if (numerical && !hex && !cmark_isdigit(data[i])) {
        break;
    } else if (!cmark_isalnum(data[i])) {
      break;
    }
    i++;
    nonempty = true;
  }

  if (data[i] == ';' && nonempty) {
    return (i - offset + 1);
  }

  return 0;

}

static const unsigned char *S_lookup(int i, int low, int hi,
                                     const unsigned char *s, int len) {
  int j;
  int cmp =
      strncmp((const char *)s, (const char *)cmark_entities[i].entity, len);
  if (cmp == 0 && cmark_entities[i].entity[len] == 0) {
    return (const unsigned char *)cmark_entities[i].bytes;
  } else if (cmp <= 0 && i > low) {
    j = i - ((i - low) / 2);
    if (j == i)
      j -= 1;
    return S_lookup(j, low, i - 1, s, len);
  } else if (cmp > 0 && i < hi) {
    j = i + ((hi - i) / 2);
    if (j == i)
      j += 1;
    return S_lookup(j, i + 1, hi, s, len);
  } else {
    return NULL;
  }
}

const unsigned char *cmark_lookup_entity(const unsigned char *s, int len) {
  return S_lookup(CMARK_NUM_ENTITIES / 2, 0, CMARK_NUM_ENTITIES - 1, s, len);
}

