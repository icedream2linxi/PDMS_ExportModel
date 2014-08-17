#pragma once
#include <string>
#include <osg/ref_ptr>
#include <osg/Group>

#ifdef __cplusplus_cli

void NetLoad(osg::ref_ptr<osg::Group> &root, const std::string &filePath);

#endif // __cplusplus_cli