// Copyright (c) 2013, Cloudera, inc
#ifndef KUDU_UTIL_MONOTIME_H
#define KUDU_UTIL_MONOTIME_H

#include <stdint.h>
#include <string>

#include <gtest/gtest.h>

namespace kudu {
class MonoTime;

// Represent an elapsed duration of time -- i.e the delta between
// two MonoTime instances.
class MonoDelta {
 public:
  static MonoDelta FromSeconds(double seconds);
  static MonoDelta FromMilliseconds(int64_t ms);
  static MonoDelta FromMicroseconds(int64_t us);
  static MonoDelta FromNanoseconds(int64_t ns);
  explicit MonoDelta();
  bool LessThan(const MonoDelta &rhs) const;
  bool MoreThan(const MonoDelta &rhs) const;
  bool Equals(const MonoDelta &rhs) const;
  std::string ToString() const;
  double ToSeconds() const;
  int64_t ToMicroseconds() const;
  int64_t ToNanoseconds() const;

  // Update struct timeval to current value of delta, with microsecond accuracy.
  // Note that if MonoDelta::IsPositive() returns true, the struct timeval
  // is guaranteed to hold a positive number as well (at least 1 microsecond).
  void ToTimeVal(struct timeval *tv) const;

  // Update struct timespec to current value of delta, with nanosecond accuracy.
  void ToTimeSpec(struct timespec *ts) const;

 private:
  friend class MonoTime;
  FRIEND_TEST(TestMonoTime, TestDeltaConversions);
  explicit MonoDelta(int64_t delta);
  int64_t nano_delta_;
};

// Represent a particular point in time, relative to some fixed but unspecified
// reference point.
//
// This time is monotonic, meaning that if the user changes his or her system
// clock, the monotime does not change.
class MonoTime {
 public:
  enum Granularity {
    COARSE,
    FINE
  };

  // The coarse monotonic time is faster to retrieve, but "only"
  // accurate to within a millisecond or two.  The speed difference will
  // depend on your timer hardware.
  static MonoTime Now(enum Granularity granularity);

  // Return MonoTime equal to farthest possible time into the future.
  static MonoTime Max();

  MonoTime();
  bool Initialized() const;
  MonoDelta GetDeltaSince(const MonoTime &rhs) const;
  void AddDelta(const MonoDelta &delta);
  bool ComesBefore(const MonoTime &rhs) const;
  std::string ToString() const;
 private:
  friend class MonoDelta;
  FRIEND_TEST(TestMonoTime, TestTimeSpec);
  FRIEND_TEST(TestMonoTime, TestDeltaConversions);
  static const int64_t kNanosecondsPerSecond = 1000000000L;
  static const int64_t kNanosecondsPerMillisecond = 1000000L;
  static const int64_t kNanosecondsPerMicrosecond = 1000L;

  explicit MonoTime(const struct timespec &ts);
  explicit MonoTime(int64_t nanos);
  double ToSeconds() const;
  uint64_t nanos_;
};
} // namespace kudu

#endif
