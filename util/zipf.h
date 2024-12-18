//  Copyright (c) 2024 Kioxia Corporation.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <math.h>

static double zeta(long n, double theta) {
  double sum = 0;
  for (long i = 0; i < n; i++) {
    sum += 1 / pow(i + 1, theta);
  }
  return sum;
}

class Zipfian {
 public:
  Zipfian(long num_items, double theta)
      : num_items_(num_items),
        theta_(theta) {
    alpha_ = 1.0 / (1.0 - theta_);
    zetan_ = zeta(num_items_, theta_);
    eta_ = (1 - pow(2.0 / num_items_, 1.0 - theta_)) / (1.0 - zeta(2, theta_) / zetan_);
  };

  long get(double z) {
    double uz = z * zetan_;
    if (uz < 1.0) {
      return 0;
    } else if (uz < 1.0 + pow(0.5, theta_)) {
      return 1;
    } else {
      return (long) ((num_items_) * pow(eta_ * z - eta_ + 1.0, alpha_));
    }
  };

 private:
  long num_items_;
  double theta_, alpha_, zetan_, eta_;
};
