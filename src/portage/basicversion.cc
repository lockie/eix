// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin V�th <vaeth@mathematik.uni-wuerzburg.de>

#include "basicversion.h"

#include <iostream>
#include <eixTk/stringutils.h>

using namespace std;

void
LeadNum::set_flags()
{
	m_is_magic = false;
	for(std::string::const_iterator it = m_text.begin();
		it != m_text.end(); ++it) {
		if(*it != '0') {
			m_is_zero = false;
			return;
		}
	}
	m_is_zero = true;
}

const char *
LeadNum::parse(const char *str)
{
	m_is_magic = false;
	const char *s = str;
	bool iszero = true;
	char c = *s;
	while(isdigit(c)) {
		if(c != '0')
			iszero = false;
		c = *(++s);
	}
	m_is_zero = iszero;
	m_text = string(str, s);
	return s;
}

int
LeadNum::compare(const LeadNum& left, const LeadNum &right)
{
	/* If you modify this, do not forget that the magic value must be
	*  the smallest one. */

	// We change the order for speed: Common cases first, if possible.
	if(left.m_is_zero) {
		if(!right.m_is_zero)
			return -1;
		// both values are 0:
		if(left.m_is_magic) {
			if(right.m_is_magic)
				return 0;
			return -1;
		}
		if(right.m_is_magic)
			return 1;
		string::size_type l = left.size();
		string::size_type r = right.size();
		if(l == r)
			return 0;
		// "" < "0" < "00" < "000" < ...
		if(l < r)
			return -1;
		return 1;
	}
	if(right.m_is_zero)
		return 1;

	// both values are nonzero:
	if(left.leadzero()) {
		if(right.leadzero())
			return strcmp(left.c_str(), right.c_str());
		return -1;
	}
	if(right.leadzero())
		return 1;

	// both values are without leading zero:
	string::size_type l = left.size();
	string::size_type r = right.size();
	if(l == r)
		return strcmp(left.c_str(), right.c_str());
	if(l < r)
		return -1;
	return 1;
}

const char *Suffix::suffixlevels[]             = { "alpha", "beta", "pre", "rc", "", "p" };
const Suffix::Level Suffix::no_suffixlevel     = 4;
const Suffix::Level Suffix::suffix_level_count = sizeof(suffixlevels)/sizeof(char*);

void
Suffix::defaults()
{
	m_suffixlevel = no_suffixlevel;
	m_suffixnum.clear();
}

string
Suffix::toString() const
{
	string ret = "_";
	ret.append(suffixlevels[m_suffixlevel]);
	ret.append(m_suffixnum.c_str());
	return ret;
}

bool
Suffix::parse(const char **str_ref)
{
	const char *str = *str_ref;
	if(*str == '_')
	{
		++str;
		for(Level i = 0; i != suffix_level_count; ++i)
		{
			if(i != no_suffixlevel
			   && strncmp(suffixlevels[i], str, strlen(suffixlevels[i])) == 0)
			{
				m_suffixlevel = i;
				str += strlen(suffixlevels[i]);
				// get suffix-level number .. "_pre123"
				*str_ref = m_suffixnum.parse(str);
				return true;
			}
		}
	}
	defaults();
	return false;
}

int
Suffix::compare(const Suffix &b) const
{
	if( m_suffixlevel < b.m_suffixlevel ) return -1;
	if( m_suffixlevel > b.m_suffixlevel ) return  1;

	if( m_suffixnum < b.m_suffixnum ) return -1;
	if( m_suffixnum > b.m_suffixnum ) return  1;

	return 0;
}

BasicVersion::BasicVersion(const char *str)
{
	if(str)
	{
		parseVersion(str);
	}
}

void
BasicVersion::defaults()
{
	m_full.clear();
	m_garbage.clear();
	m_primsplit.clear();
	m_primarychar   = '\0';
	m_suffix.clear();
	m_gentoorevision.set_magic();
	m_subrevision.set_magic();
}

void
BasicVersion::calc_full()
{
	m_full.empty();
	bool first = true;
	for(vector<LeadNum>::const_iterator it = m_primsplit.begin();
		it != m_primsplit.end(); ++it) {
		if(!first) {
			m_full.append(".");
		}
		first = false;
		m_full.append(it->c_str());
	}
	if(m_primarychar)
		m_full.append(string(1,m_primarychar));
	for(vector<Suffix>::const_iterator i = m_suffix.begin();
		i != m_suffix.end(); ++i) {
		m_full.append(i->toString());
	}
	if(!m_gentoorevision.is_magic()) {
		m_full.append("-r");
		m_full.append(m_gentoorevision.c_str());
		if(!m_subrevision.is_magic()) {
			m_full.append(".");
			m_full.append(m_subrevision.c_str());
		}
	}
	m_full.append(m_garbage);
}

void
BasicVersion::parseVersion(const char *str, size_t n)
{
	defaults();
	if(n > 0)
		m_full = string(str, n);
	else
		m_full = string(str);
	str = parsePrimary(m_full.c_str());
	if(*str)
	{
		Suffix curr_suffix;
		while(curr_suffix.parse(&str))
			m_suffix.push_back(curr_suffix);

		// get optional gentoo revision
		if(!strncmp("-r", str, 2))
		{
			str = m_gentoorevision.parse(str + 2);
			if((*str == '.') && m_gentoorevision.leadzero())
				str = m_subrevision.parse(str + 1);
		}
		if(*str != '\0')
		{
			m_garbage = str;
			cerr << "Garbage at end of version string: " << str << endl;
		}
	}
}

/** Compares the split m_primsplit numbers of another BasicVersion instances to itself. */
int BasicVersion::comparePrimary(const BasicVersion& left,
		const BasicVersion& right)
{
	vector<LeadNum>::const_iterator ait = left.m_primsplit.begin();
	vector<LeadNum>::const_iterator bit = right.m_primsplit.begin();
	for( ; (ait != left.m_primsplit.end()) && (bit != right.m_primsplit.end());
		++ait, ++bit)
	{
		if(*ait < *bit)
			return -1;
		if(*ait > *bit)
			return 1;
	}
	/* The one with the bigger amount of versionsplits is our winner */
	if(ait != left.m_primsplit.end())
		return 1;
	if(bit != right.m_primsplit.end())
		return -1;
	return 0;
}

/** Compares the split m_suffixes of another BasicVersion instances to itself. */
int BasicVersion::compareSuffix(const BasicVersion& left,
		const BasicVersion& right)
{
	vector<Suffix>::const_iterator ait = left.m_suffix.begin();
	vector<Suffix>::const_iterator bit = right.m_suffix.begin();
	for( ; (ait != left.m_suffix.end()) && (bit != right.m_suffix.end());
		++ait, ++bit)
	{
		int ret = ait->compare(*bit);
		if(ret)
			return ret;
	}
	static const Suffix empty;
	const Suffix &aref = (ait == left.m_suffix.end()) ? empty : *ait;
	const Suffix &bref = (bit == right.m_suffix.end()) ? empty : *bit;
	return aref.compare(bref);
}

int
BasicVersion::compareTilde(const BasicVersion& left, const BasicVersion &right)
{
	int ret = comparePrimary(left, right);
	if(ret) return ret;

	if( left.m_primarychar < right.m_primarychar ) return -1;
	if( left.m_primarychar > right.m_primarychar ) return  1;
	return compareSuffix(left, right);
}

int
BasicVersion::compare(const BasicVersion& left, const BasicVersion &right)
{
	int ret = compareTilde(left, right);
	if(ret) return ret;

	const LeadNum *leftrev, *rightrev;
	bool new_left, new_right;
	if( left.m_subrevision.is_magic() || !left.m_gentoorevision.leadzero()) {
		new_left = false;
		leftrev = &(left.m_gentoorevision);
	}
	else {
		new_left = true;
		leftrev = new LeadNum(string(left.m_gentoorevision.c_str() + 1));
	}
	if( right.m_subrevision.is_magic() || !right.m_gentoorevision.leadzero()) {
		new_right = false;
		rightrev = &(right.m_gentoorevision);
	}
	else {
		new_right = true;
		rightrev = new LeadNum(string(right.m_gentoorevision.c_str() + 1));
	}
	if( (*leftrev) < (*rightrev) ) ret = -1;
	if( (*leftrev) > (*rightrev) ) ret = 1;
	if(new_left) delete leftrev;
	if(new_right) delete rightrev;
	if(ret) return ret;

	if( left.m_subrevision < right.m_subrevision ) return -1;
	if( left.m_subrevision > right.m_subrevision ) return 1;

	// The numbers are equal, but the strings might still be different,
	// e.g. because of garbage or removed trailing ".0"s.
	// In such a case, we simply compare the strings alphabetically.
	// This is not always what you want but at least reproducible.
	return strcmp(left.m_garbage.c_str(), right.m_garbage.c_str());
}

const char *
BasicVersion::parsePrimary(const char *str)
{
	string buf;
	while(*str)
	{
		if(*str == '.') {
			m_primsplit.push_back(LeadNum(buf));
			buf.clear();
		}
		else if(isdigit(*str))
			buf.push_back(*str);
		else
			break;
		++str;
	}

	if(!buf.empty())
		m_primsplit.push_back(LeadNum(buf));

	if(isalpha(*str))
		m_primarychar = *str++;
	return str;
}

const ExtendedVersion::Restrict
	ExtendedVersion::RESTRICT_NONE,
	ExtendedVersion::RESTRICT_FETCH,
	ExtendedVersion::RESTRICT_MIRROR;

ExtendedVersion::Restrict
ExtendedVersion::calcRestrict(const string &str)
{
	Restrict r = RESTRICT_NONE;
	vector<string> restrict_words = split_string(str);
	for(vector<string>::const_iterator it = restrict_words.begin();
		it != restrict_words.end(); ++it) {
		if(strcasecmp(it->c_str(), "fetch") == 0)
			r |= RESTRICT_FETCH;
		else if(strcasecmp(it->c_str(), "mirror") == 0)
			r |= RESTRICT_MIRROR;
	}
	return r;
}
