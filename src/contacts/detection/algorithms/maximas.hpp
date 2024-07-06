// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_MAXIMAS_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_MAXIMAS_HPP

#include <common/types.hpp>

#include <vector>

namespace iptsd::contacts::detection::maximas {

template <class Derived>
void check_point(const DenseBase<Derived> &data, std::vector<Point> &maximas, Eigen::Index y, Eigen::Index x, Eigen::Index cols, Eigen::Index rows)
{
	using T = typename DenseBase<Derived>::Scalar;
	const T value = data(y, x);
	bool max = true;

	const bool can_up = y > 0;
	const bool can_down = y < rows;
	
	const bool can_left = x > 0;
	const bool can_right = x < cols;
	
	if (can_left)
		max &= data(y, x - 1) < value;

	if (can_right)
		max &= data(y, x + 1) <= value;

	if (can_up) {
		max &= data(y - 1, x) < value;

		if (can_left)
			max &= data(y - 1, x - 1) < value;

		if (can_right)
			max &= data(y - 1, x + 1) <= value;
	}

	if (can_down) {
		max &= data(y + 1, x) <= value;

		if (can_left)
			max &= data(y + 1, x - 1) < value;

		if (can_right)
			max &= data(y + 1, x + 1) <= value;
	}

	if (max)
		maximas.emplace_back(x, y);
}

/*!
 * Searches for all local maxima in the given data.
 *
 * @param[in] data The data to process.
 * @param[in] threshold Only return local maxima whose value is above this threshold.
 * @param[out] maximas A reference to the vector where the found points will be stored.
 */
template <class Derived>
void find(const DenseBase<Derived> &data,
          typename DenseBase<Derived>::Scalar threshold,
          std::vector<Point> &maximas)
{

	/*
	 * We use the following kernel to compare entries:
	 *
	 *   [< ] [< ] [< ]
	 *   [< ] [  ] [<=]
	 *   [<=] [<=] [<=]
	 *
	 * Half of the entries use "less or equal", the other half "less than" as
	 * operators to ensure that we don't either discard any local maximas or
	 * report some multiple times.
	 */

	const Eigen::Index cols = data.cols() - 1;
	const Eigen::Index rows = data.rows() - 1;

	maximas.clear();
	
	Eigen::Index y_up = 0;
	Eigen::Index y_down = rows;
	
	Eigen::Index x_left = 0;
	Eigen::Index x_right = cols;
	
	while (y_up < y_down) {
		
		if (x_left >= x_right) {
			y_up++;
			y_down--;
			x_left = 0;
			x_right = cols;
			continue;
		}
		
		if (data(y_up, x_left) > threshold) {
			
			check_point(data, maximas, y_up, x_left, cols, rows);
		}
		if (data(y_up, x_right) > threshold) {
		
			check_point(data, maximas, y_up, x_right, cols, rows);
		}
		if (data(y_down, x_left) > threshold) {
		
			check_point(data, maximas, y_down, x_left, cols, rows);
		}
		if (data(y_down, x_right) > threshold) {
		
			check_point(data, maximas, y_down, x_right, cols, rows);
		}
		
		x_left++;
		x_right--;
	}
}

} // namespace iptsd::contacts::detection::maximas

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_MAXIMAS_HPP
