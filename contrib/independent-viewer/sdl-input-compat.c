/* Local SDL2 adapter for uu-remote-linux 0.2.45's repeat:false filter.
 * Uses the SDL 2.32 event ABI; never logs events or clipboard data.
 * Repeated Backspace becomes balanced release/press pairs. Optional PC-style
 * editing chords temporarily use Command, preserving other Control chords.
 * Loaded only by the independent Mac entry, never by the main application.
 */
typedef unsigned int Uint32;
typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef struct {
    int scancode, sym;
    Uint16 mod;
    Uint32 unused;
} Keysym;
typedef struct {
    Uint32 type, timestamp, window_id;
    Uint8 state, repeat, padding2, padding3;
    Keysym keysym;
} KeyboardEvent;
typedef union {
    Uint32 type;
    KeyboardEvent key;
    unsigned char padding[56];
    void *alignment;
} Event;
typedef char event_size_must_be_56[(sizeof(Event) == 56) ? 1 : -1];
extern void *dlsym(void *, const char *);
extern char *getenv(const char *);

static Event queued[16];
static int queued_count, queued_next;
static int physical_ctrl, sent_ctrl, physical_gui, sent_gui;
static unsigned char mapped[512];
static int mapped_count;

static int flag(const char *name) {
    const char *value = getenv(name);
    return value && value[0] == '1' && !value[1];
}
static void emit(Event event) { queued[queued_count++] = event; }
static void modifier(Event source, int scan, int down) {
    source.key.type = down ? 0x300 : 0x301;
    source.key.state = down;
    source.key.repeat = 0;
    source.key.keysym.scancode = scan;
    source.key.keysym.sym = (1 << 30) | scan;
    emit(source);
}
static void release_gui(Event source) {
    if (sent_gui && !(physical_gui & 1))
        modifier(source, 227, 0);
    sent_gui = 0;
}
static void restore_ctrl(Event source) {
    int bit;
    for (bit = 1; bit <= 2; bit <<= 1) {
        if ((physical_ctrl & bit) && !(sent_ctrl & bit))
            modifier(source, bit == 1 ? 224 : 228, 1);
    }
    sent_ctrl = physical_ctrl;
}
static int edit_key(int scan) {
    /* A C V X Z F S O N P W L T R. No Ctrl+Alt or physical Win chords. */
    switch (scan) {
        case 4: case 6: case 25: case 27: case 29: case 9: case 22:
        case 18: case 17: case 19: case 26: case 15: case 23: case 21:
            return 1;
    }
    return 0;
}
static void reset(void) {
    int i;
    physical_ctrl = sent_ctrl = physical_gui = sent_gui = mapped_count = 0;
    for (i = 0; i < 512; ++i) mapped[i] = 0;
}
static void adapt(Event source, int shortcuts, int repeat) {
    int scan, down, bit, i;
    /* Focus loss is followed by the stock viewer's release_all(). */
    if (source.type == 0x100 || (source.type == 0x200 &&
        (source.padding[12] == 13 || source.padding[12] == 14))) {
        reset(); emit(source); return;
    }
    if (source.type != 0x300 && source.type != 0x301) {
        emit(source); return;
    }
    scan = source.key.keysym.scancode;
    down = source.type == 0x300;
    if (shortcuts) {
        if (scan == 224 || scan == 228) {
            bit = scan == 224 ? 1 : 2;
            if (down) {
                physical_ctrl |= bit;
                if (!sent_gui && !(sent_ctrl & bit)) {
                    sent_ctrl |= bit; emit(source);
                }
            } else {
                physical_ctrl &= ~bit;
                if (sent_ctrl & bit) { sent_ctrl &= ~bit; emit(source); }
                if (!physical_ctrl) release_gui(source);
            }
            return;
        }
        if (scan == 227 || scan == 231) {
            bit = scan == 227 ? 1 : 2;
            if (down) physical_gui |= bit; else physical_gui &= ~bit;
            /* The logical left Command is already down for a mapped chord. */
            if (scan != 227 || !sent_gui) emit(source);
            return;
        }
        if (down && !source.key.repeat && edit_key(scan) &&
            (source.key.keysym.mod & 0x00c0) && !(source.key.keysym.mod & 0x0f00)) {
            for (i = 1; i <= 2; i <<= 1)
                if (sent_ctrl & i) modifier(source, i == 1 ? 224 : 228, 0);
            sent_ctrl = 0;
            if (!sent_gui) {
                modifier(source, 227, 1);
                sent_gui = 1;
            }
            if (!mapped[scan]) { mapped[scan] = 1; ++mapped_count; }
            source.key.keysym.mod = (source.key.keysym.mod & ~0x00c0) | 0x0400;
            emit(source); return;
        }
        if (!down && scan >= 0 && scan < 512 && mapped[scan]) {
            mapped[scan] = 0; --mapped_count;
            emit(source);
            if (!mapped_count) { release_gui(source); restore_ctrl(source); }
            return;
        }
        /* A different chord (e.g. Ctrl+Alt+F) needs the real Control keys. */
        if (down && !source.key.repeat && sent_gui &&
            scan != 225 && scan != 229) {
            release_gui(source); restore_ctrl(source);
        }
    }
    if (repeat && down && source.key.repeat && scan == 42) {
        Event released = source;
        released.key.type = 0x301;
        released.key.state = released.key.repeat = 0;
        emit(released);
        source.key.repeat = 0;
    }
    emit(source);
}

int SDL_PollEvent(Event *event) {
    typedef int (*Poll)(Event *);
    static Poll poll;
    static int repeat = -1, shortcuts;
    if (!poll)
        poll = (Poll)dlsym((void *)-1, "SDL_PollEvent");
    if (!poll)
        return 0;
    if (repeat < 0) {
        repeat = flag("UU_VIEWER_BACKSPACE_REPEAT");
        shortcuts = flag("UU_VIEWER_PC_SHORTCUTS");
    }
    if (!repeat && !shortcuts)
        return poll(event);
    while (queued_next == queued_count) {
        Event source;
        queued_next = queued_count = 0;
        if (!event) return poll(event);
        if (!poll(&source)) return 0;
        adapt(source, shortcuts, repeat);
    }
    if (event) {
        *event = queued[queued_next++];
        if (queued_next == queued_count) {
            queued_next = queued_count = 0;
        }
    }
    return 1;
}
