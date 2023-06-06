// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_STABILITY_STABILIZER_HPP
#define IPTSD_CONTACTS_STABILITY_STABILIZER_HPP

#include "../contact.hpp"
#include "config.hpp"

#include <common/casts.hpp>
#include <common/constants.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <algorithm>
#include <deque>
#include <iterator>
#include <type_traits>
#include <vector>

namespace iptsd::contacts::stability {

template <class T>
class Stabilizer {
public:
	static_assert(std::is_floating_point_v<T>);

private:
	Config<T> m_config;

	// The last n frames, with n being m_config.temporal_window.
	std::deque<std::vector<Contact<T>>> m_frames;

public:
	Stabilizer(Config<T> config)
		: m_config {config}
		, m_frames {std::max(config.temporal_window, casts::to<usize>(2) - 1)} {};

	/*!
	 * Resets the stabilizer by clearing the stored copies of the last frames.
	 */
	void reset()
	{
		for (auto &frame : m_frames)
			frame.clear();
	}

	/*!
	 * Stabilizes all contacts of a frame.
	 *
	 * @param[in,out] frame The list of contacts to stabilize.
	 */
	void stabilize(std::vector<Contact<T>> &frame)
	{
		// Stabilize contacts
		for (Contact<T> &contact : frame)
			this->stabilize_contact(contact, m_frames.back());

		auto nf = m_frames.front();

		// Clear the oldest stored frame
		m_frames.pop_front();
		nf.clear();

		// Copy the new frame
		std::copy(frame.begin(), frame.end(), std::back_inserter(nf));
		m_frames.push_back(nf);
	}

private:
	/*!
	 * Stabilize a single contact.
	 *
	 * @param[in,out] contact The contact to stabilize.
	 * @param[in] frame The previous frame.
	 */
	void stabilize_contact(Contact<T> &contact, const std::vector<Contact<T>> &frame) const
	{
		// Contacts that can't be tracked can't be stabilized.
		if (!contact.index.has_value())
			return;

		if (m_config.check_temporal_stability && m_config.temporal_window >= 2)
			contact.stable = this->check_temporal(contact);
		else
			contact.stable = true;

		if (m_config.temporal_window < 2)
			return;

		const usize index = contact.index.value();
		const auto wrapper = Contact<T>::find_in_frame(index, frame);

		if (!wrapper.has_value())
			return;

		const Contact<T> &last = wrapper.value();

		if (m_config.size_threshold.has_value())
			this->stabilize_size(contact, last);

		if (m_config.position_threshold.has_value())
			this->stabilize_position(contact, last);

		if (m_config.orientation_threshold.has_value())
			this->stabilize_orientation(contact, last);
	}

	/*!
	 * Checks the temporal stability of a contact.
	 *
	 * A contact is temporally stable if it appears in all frames of the temporal window.
	 *
	 * @param[in] contact The contact to check.
	 * @return Whether the contact is present in all previous frames.
	 */
	[[nodiscard]] bool check_temporal(const Contact<T> &contact) const
	{
		// Contacts that can't be tracked are considered temporally stable.
		if (!contact.index.has_value())
			return true;

		const usize index = contact.index.value();

		// Iterate over the last frames and find the contact with the same index
		for (auto itr = m_frames.crbegin(); itr != m_frames.crend(); itr++) {
			const auto wrapper = Contact<T>::find_in_frame(index, *itr);

			if (!wrapper.has_value())
				return false;
		}

		return true;
	}

	/*!
	 * Stabilizes the size of the contact.
	 *
	 * @param[in,out] current The contact to stabilize.
	 * @param[in] last The contact to compare against.
	 */
	void stabilize_size(Contact<T> &current, const Contact<T> &last) const
	{
		if (!m_config.size_threshold.has_value())
			return;

		const Vector2<T> thresh = m_config.size_threshold.value();
		const Vector2<T> delta = (current.size - last.size).cwiseAbs();

		/*
		 * If the size is increasing too slow, discard the change.
		 * If the size is increasing too fast, mark it as unstable (we can't stabilize it).
		 * Otherwise, don't change the size.
		 */

		if (delta.x() < thresh.x())
			current.size.x() = last.size.x();
		else if (delta.x() > thresh.y())
			current.stable = false;

		if (delta.y() < thresh.x())
			current.size.y() = last.size.y();
		else if (delta.y() > thresh.y())
			current.stable = false;
	}

	/*!
	 * Stabilizes the position of the contact.
	 *
	 * @param[in,out] current The contact to stabilize.
	 * @param[in] last The contact to compare against.
	 */
	void stabilize_position(Contact<T> &current, const Contact<T> &last) const
	{
		if (!m_config.position_threshold.has_value())
			return;

		const Vector2<T> thresh = m_config.position_threshold.value();

		const Vector2<T> delta = current.mean - last.mean;
		const T distance = std::hypot(delta.x(), delta.y());

		/*
		 * If the contact is moving too slow, discard the position change.
		 * If the contact is moving too fast, mark it as unstable (we can't stabilize it).
		 * Otherwise, don't change the position.
		 */

		if (distance < thresh.x())
			current.mean = last.mean;
		else if (distance > thresh.y())
			current.stable = false;
	}

	void stabilize_orientation(Contact<T> &current, const Contact<T> &last) const
	{
		if (!m_config.orientation_threshold.has_value())
			return;

		const T aspect = current.size.maxCoeff() / current.size.minCoeff();

		/*
		 * If the aspect ratio is too small, the orientation cannot be determined
		 * in a stable way. To prevent errors, we set it to 0 in this case.
		 *
		 * TODO: Check if there is a better way to signal this (make orientation optional,
		 * and / or applying the last stable value).
		 */
		if (aspect < 1.1) {
			current.orientation = 0;
			return;
		}

		const Vector2<T> thresh = m_config.orientation_threshold.value();

		const T max = current.normalized ? One<T>() : gsl::narrow_cast<T>(M_PI);

		// The angle difference in both directions.
		const T d1 = std::abs(current.orientation - last.orientation);
		const T d2 = max - d1;

		// Pick the smaller difference to properly handle going from 0° to 179°.
		const T delta = std::min(d1, d2);

		/*
		 * If the angle is changing too slow, discard the orientation change.
		 * If the angle is changing too fast, mark it as unstable (we can't stabilize it).
		 * Otherwise, don't change the orientation.
		 */

		if (delta < thresh.x())
			current.orientation = last.orientation;
		else if (delta > thresh.y())
			current.stable = false;
	}
};

} // namespace iptsd::contacts::stability

#endif // IPTSD_CONTACTS_STABILITY_STABILIZER_HPP