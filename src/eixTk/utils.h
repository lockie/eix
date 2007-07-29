/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin V�th <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <map>
#include <vector>
#include <string>

/** push_back every line of file or dir into v. */
bool pushback_lines(const char *file, std::vector<std::string> *v, bool remove_empty = true, bool recursive = false, bool remove_comments = true);

/** List of files in directory.
 * Pushed names of file in directory into string-vector if the don't match any
 * char * in given exlude list.
 * @param dir_path Path to directory
 * @param into pointer to vector of strings .. files get append here (with full path)
 * @param exlude list of char * that don't need to be put into vector
 * @param only_type: if 1: consider only ordinary files, if 2: consider only dirs, if 3: consider only files or dirs
 * @param no_hidden ignore hidden files
 * @param full_path return full pathnames
 * @return true if everything is ok */
bool pushback_files(const std::string &dir_path, std::vector<std::string> &into, const char *exclude[] = NULL, short only_files = 1, bool no_hidden = true, bool full_path = true);


/** Cycle through map using it, until it is it_end, append all values from it
 * to the value with the same key in append_to. */
void join_map(std::map<std::string,std::string> *append_to, std::map<std::string,std::string>::iterator it, std::map<std::string,std::string>::iterator it_end);

/** A percent status for stdout.
 * Only shows a status */
class PercentStatus {
	public:
		/** Calculacte step-width.
		 * @param max number of steps you want to do. */
		PercentStatus(unsigned int max = 0)
		{
			m_max = m_run = 0;
			m_step_size = 0;
			if(max != 0) {
				start(max);
			}
		}

		/** Start status. */
		void start(unsigned int max)
		{
			m_max = max;
			m_run = 0;
			m_step_size = 100.0/max;
			printf("  0%%");
		}

		/** Print next step. */
		void operator ++()
		{
			if(++m_run == m_max)
			{
				puts("\b\b\b\b" "100%");
				fflush(stdout);
				return;
			}

			printf("\b\b\b\b" "%.3i%%", int(m_run * m_step_size));
			fflush(stdout);
		}

	protected:
		unsigned int m_max, /**<Number of steps. */
					 m_run;
		float m_step_size;  /**< Step-size. */
};

/* Print version of eix to stdout. */
void dump_version(int exit_code = -1);


/** Sort and unique. Return true if there were double entries */
#ifdef UNIQUE_WORKS
template<typename T>
bool sort_uniquify(T &v, bool vector_is_ignored = false)
{
	std::sort(v.begin(), v.end());
	typename T::iterator i = std::unique(v.begin(), v.end());
	if(i == v.end())
		return false;
	if(vector_is_ignored)
		return true;
	v.erase(i, v.end());
	return true;
}

#else
#include <set>

template<typename T>
bool sort_uniquify(std::vector<T> &v, bool vector_is_ignored = false)
{
	std::set<T> s;
	for(typename std::vector<T>::const_iterator i = v.begin();
		i != v.end(); ++i)
		s.insert(*i);
	bool r = (s.size() != v.size());
	if(vector_is_ignored)
		return r;
	v.clear();
	for(typename std::set<T>::const_iterator i = s.begin();
		i != s.end(); ++i)
		v.push_back(*i);
	return r;
}

#endif


#endif /* __UTILS_H__ */
