#pragma once
#include <string>
#include <osg/ref_ptr>
#include <osg/Group>

void NetLoad(osg::ref_ptr<osg::Group> &root, const std::string &filePath);