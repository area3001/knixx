#ifndef LOG_H_
#define LOG_H_

#define LOG_STR_MAX	80

enum log_lvl {
	LOG_LVL_TRACE,
	LOG_LVL_DEBUG,
	LOG_LVL_INFO,
	LOG_LVL_WARN,
	LOG_LVL_ERROR,
	LOG_LVL_OFF
};

enum log_topic {
	LOG_TOPIC_BUTTON,
	LOG_TOPIC_NCN
};

void log_write(enum log_topic topic, enum log_lvl lvl, const char *fmt, ...);

#endif
