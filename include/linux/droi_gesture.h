#ifndef __DROI_GESTURE__
#define __DROI_GESTURE__

#define REPORT_TRIGGER
typedef int dg_enable_t(int);
typedef int dg_getdata_t(void);

enum{
	GES_LEFT=1,
	GES_RIGHT,
	GES_UP,
	GES_DOWN,
	GES_NON_EFFECT = 0,
};

struct sensor_ops{
	dg_enable_t		*enable;
	dg_getdata_t	*getdata;
};
extern int	droi_gesture_ops_register(struct sensor_ops *ops);
extern void droi_gestrue_trigger(int ge);
extern int droi_gesture_input_register(struct input_dev *dev);
#endif //#ifndef __DROI_GESTURE__
