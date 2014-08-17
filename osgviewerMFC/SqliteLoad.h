#pragma once
#include <string>
#include <osg/ref_ptr>
#include <osg/Group>
#include "sqlite3.h"

class SqliteLoad
{
public:
	SqliteLoad(osg::ref_ptr<osg::Group> &root, const std::wstring &filePath);
	~SqliteLoad();
	bool doLoad();
	const wchar_t *getErrorMessage() const;

private:
	int init();
	bool loadBox();

private:
	std::wstring m_filePath;
	osg::ref_ptr<osg::Group> m_root;

	sqlite3 *m_pDb;
	int m_errorCode;
};