//
// Copyright © 2018 Roman Sobkuliak <r.sobkuliak@gmail.com>
// This code is released under the license described in the LICENSE file
// 

#include <stdexcept>
#include <memory>


class HranolException : public std::exception
{
protected:
	// Error message
	std::string msg_;

public:
	explicit HranolException(const char* message) :
		msg_(message)
	{}
	explicit HranolException(const std::string& message) :
		msg_(message)
	{}

	virtual ~HranolException() throw () {}

	virtual const char* what() const throw () {
		return msg_.c_str();
	}

	// Append method allows appending text to exception message
	void append(const char* msg) {
		msg_.append(msg);
	}

	// Using pass-by-value because message has to be copied anyway
	// This way call with std::move can be used. E.g.: e.append(std::move(s))
	void append(std::string message) {
		msg_.append(std::move(message));
	}
};

class HranolRuntimeException : public HranolException {
public:
	explicit HranolRuntimeException(const char* message)
		: HranolException(message)
	{}
	explicit HranolRuntimeException(const std::string& message)
		: HranolException(message)
	{}
};