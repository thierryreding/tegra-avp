#ifndef AVP_BOARD_H
#define AVP_BOARD_H

struct board {
	void (*power_off)(void);
};

#define board_init(b)	\
	static struct board *__##b __attribute__((section(".board.init"))) \
		__attribute__((used)) = &b;

#endif /* AVP_BOARD_H */
