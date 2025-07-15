#ifndef _SOUND_H_
#define _SOUND_H_

#include <string>
#include <codecvt>
#include <graphics.h>
#include <mmsystem.h>

#pragma comment(lib, "Winmm.lib")

class Sound
{
public:
	typedef size_t ID;
	static const ID INVALID;

public:
	static Sound* Instance()
	{
		if (!instance)
			instance = new Sound();

		return instance;
	}

	~Sound()
	{
	}

	ID Load(const std::string& path)
	{
		ID id = next_id++;

		static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		std::wstring str_cmd = _T("open ") + converter.from_bytes(path) + _T(" alias ") + std::to_wstring(id);
		MCIERROR err = mciSendString(str_cmd.c_str(), NULL, 0, NULL);

		return err ? INVALID : id;
	}

	void Play(ID id, bool loop = true)
	{
		std::wstring str_cmd = _T("play ") + std::to_wstring(id) + (loop ? _T(" repeat from 0") : _T(" from 0"));
		mciSendString(str_cmd.c_str(), NULL, 0, NULL);
	}

	void Stop(ID id)
	{
		std::wstring str_cmd = _T("stop ") + std::to_wstring(id);
		mciSendString(str_cmd.c_str(), NULL, 0, NULL);
	}

	void Pause(ID id)
	{
		std::wstring str_cmd = _T("pause ") + std::to_wstring(id);
		mciSendString(str_cmd.c_str(), NULL, 0, NULL);
	}

	void Resume(ID id)
	{
		std::wstring str_cmd = _T("resume ") + std::to_wstring(id);
		mciSendString(str_cmd.c_str(), NULL, 0, NULL);
	}

private:
	Sound()
	{
	}

private:
	ID next_id = 0;
	static Sound* instance;
};


#endif // !_SOUND_H_
