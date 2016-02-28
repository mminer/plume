typedef struct mp_cur {
    const unsigned char *p;
    size_t left;
    int err;
} mp_cur;

void mp_cur_init(mp_cur *cursor, const unsigned char *s, size_t len);
void mp_decode_to_lua_type(lua_State *L, mp_cur *c);
