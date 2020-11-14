#include "Tag.h"

Tag::Tag(int id=0, int picID=0, int userID=0)
{
	_ID = id;
	_USER_ID = userID;
	_PICTURE_ID = picID;
}

Tag::~Tag()
{
}

int Tag::getPictureID()
{
	return _PICTURE_ID;
}

int Tag::getID()
{
	return _ID;
}

int Tag::getUserID()
{
	return _USER_ID;
}

void Tag::setPictureID(int id)
{
	_PICTURE_ID = id;
}

void Tag::setUserID(int id)
{
	_USER_ID = id;
}

void Tag::setID(int id)
{
	_ID = id;
}
