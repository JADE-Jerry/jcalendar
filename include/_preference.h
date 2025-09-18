#ifndef ___PREFERENCE_H__
#define ___PREFERENCE_H__

#include <Preferences.h>
#define PREF_NAMESPACE "J_CALENDAR"

// Preferences KEY定义
// !!!preferences key限制15字符
#define PREF_SI_CAL_DATE "SI_CAL_DATE" // 屏幕当前显示的日期
#define PREF_SI_WEEK_1ST "SI_WEEK_1ST" // 每周第一天，0: 周日（默认），1:周一
#define PREF_SI_TYPE "SI_TYPE" // 屏幕显示类型

#define PREF_QWEATHER_HOST "QWEATHER_HOST" // QWEATHER HOST
#define PREF_QWEATHER_KEY "QWEATHER_KEY" // QWEATHER KEY/TOKEN
#define PREF_QWEATHER_TYPE "QWEATHER_TYPE" // 0: 每日天气，1: 实时天气
#define PREF_QWEATHER_LOC "QWEATHER_LOC" // 地理位置
#define PREF_CD_DAY_DATE "CD_DAY_DATE" // 倒计日
#define PREF_CD_DAY_LABLE "CD_DAY_LABLE" // 倒计日名称
#define PREF_TAG_DAYS "TAG_DAYS" // tag day
#define PREF_STUDY_SCHEDULE "STUDY_SCH" // 课程表

// 假期信息，tm年，假期日(int8)，假期日(int8)...
#define PREF_HOLIDAY "HOLIDAY"

#endif
