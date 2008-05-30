#ifndef RAGEL_PARSER_MACROS
#define RAGEL_PARSER_MACROS

/* Sets the line_start variable to ts.
 * This is typically used for the SPACE entity in the main action.
 */
#define ls { if (!line_start) line_start = ts; }

/* The C equivalent of the Ragel 'code' action.
 * This is tyically used in the main action for entities where Ragel actions
 * cannot, for one reason or another, be used.
 */
#define code {\
  if (!line_contains_code && !line_start) line_start = ts; \
  line_contains_code = 1; \
}

/* The C equivalent of the Ragel 'comment' action.
 * This is typically unused, but here for consistency.
 */
#define comment {\
  if (!line_contains_code) { \
    whole_line_comment = 1; \
    if (!line_start) line_start = ts; \
  } \
}

/* Executes standard line counting actions for INTERNAL_NL entities.
 * This is typically used in the main action for the INTERNAL_NL entity.
 * @param lang The language name string.
 */
#define std_internal_newline(lang) { \
  if (callback && p > line_start) { \
    if (line_contains_code) \
      callback(lang, "lcode", cint(line_start), cint(p)); \
    else if (whole_line_comment) \
      callback(lang, "lcomment", cint(line_start), cint(p)); \
    else \
      callback(lang, "lblank", cint(line_start), cint(p)); \
  } \
  whole_line_comment = 0; \
  line_contains_code = 0; \
  line_start = p; \
}

/* Executes standard line counting actions for NEWLINE entities.
 * This is typically used in the main action for the NEWLINE entity.
 * @param lang The language name string.
 */
#define std_newline(lang) {\
  if (callback && te > line_start) { \
    if (line_contains_code) \
      callback(lang, "lcode", cint(line_start), cint(te)); \
    else if (whole_line_comment) \
      callback(lang, "lcomment", cint(line_start), cint(te)); \
    else \
      callback(lang, "lblank", cint(ts), cint(te)); \
  } \
  whole_line_comment = 0; \
  line_contains_code = 0; \
  line_start = 0; \
}

/* Processes the last line for buffers that don't have a newline at EOF.
 * This is typically used at the end of the parse_lang function after the Ragel
 * parser has been executed.
 * @param lang The language name string.
 */
#define process_last_line(lang) {\
  if ((whole_line_comment || line_contains_code) && callback) { \
    if (line_contains_code) \
      callback(lang, "lcode", cint(line_start), cint(pe)); \
    else if (whole_line_comment) \
      callback(lang, "lcomment", cint(line_start), cint(pe)); \
  } \
}

/* Determines whether or not the rest of the line is blank.
 * This is typically used when entering an embedded language.
 * @param p The position of entry into the emebedded language.
 * @return 0 if the rest of the line is not blank, the position at the end of
 *   the newline otherwise (inclusive).
 */
int is_blank_entry(char *p) {
  char *pos = p+1;
  while (*pos != '\n' && *pos != '\r' && *pos != '\f') {
    if (*pos != '\t' || *pos != ' ') return 0;
    pos++;
  }
  if (*pos == '\r' && *(pos+1) == '\n') pos++;
  return pos;
}

/* If there is a transition into an embedded language and there is only parent
 * language code on the line (the rest of the line is blank with no child code),
 * count the line as a line of parent code.
 * Moves p and te to the end of the newline and calls the std_newline macro. (p
 * is inclusive, te is not.)
 * This is typically used in the main action for the CHECK_BLANK_ENTRY entity.
 * @param lang The language name string.
 */
#define check_blank_entry(lang) { \
  if (is_blank_entry(p)) { \
    p = is_blank_entry(p); \
    te = p + 1; \
    std_newline(lang) \
  } \
}

// Variables used by all parsers. Do not modify.

// used for newlines
#define NEWLINE -1

// used for newlines inside patterns like strings and comments that can have
// newlines in them
#define INTERNAL_NL -2

#define CHECK_BLANK_ENTRY -3

// required by Ragel
int cs, act;
char *p, *pe, *eof, *ts, *te;
int stack[5], top;

// used for calculating offsets from buffer start for start and end positions
char *buffer_start;
#define cint(c) ((int) (c - buffer_start))

// state flags for line and comment counting
int whole_line_comment;
int line_contains_code;

// the beginning of a line in the buffer for line and comment counting
char *line_start;

// state variable for the current entity being matched
int entity;

#define init { \
  p = buffer; \
  pe = buffer + length; \
  eof = pe; \
  \
  buffer_start = buffer; \
  whole_line_comment = 0; \
  line_contains_code = 0; \
  line_start = 0; \
  entity = 0; \
}

#endif