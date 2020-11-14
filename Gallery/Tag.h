#pragma once
class Tag
{
public:
	Tag(int, int, int);
	~Tag();
	int getPictureID();
	int getID();
	int getUserID();
	void setPictureID(int id);
	void setUserID(int id);
	void setID(int id);


private:
	int _ID;
	int _PICTURE_ID;
	int _USER_ID;
};

