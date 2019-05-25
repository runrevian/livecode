/* Copyright (C) 2015 LiveCode Ltd.
 
 This file is part of LiveCode.
 
 LiveCode is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License v3 as published by the Free
 Software Foundation.
 
 LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

#ifndef __LIBBROWSER_INTERNAL_H__
#define __LIBBROWSER_INTERNAL_H__

#include <libbrowser.h>

////////////////////////////////////////////////////////////////////////////////

typedef bool (*MCBrowserIterateCallback)(MCBrowser *p_browser, void *p_context);

////////////////////////////////////////////////////////////////////////////////
// C++ Implementation Class

// Browser implementation base class
class MCBrowserBase : public MCBrowser
{
public:
    MCBrowserBase(void);
    virtual ~MCBrowserBase(void);

	void SetEventHandler(MCBrowserEventHandler *p_handler);
	void SetJavaScriptHandler(MCBrowserJavaScriptHandler *p_handler);
	void SetProgressHandler(MCBrowserProgressHandler *p_handler);
	void SetFileDialogHandler(MCBrowserFileDialogHandler *p_handler);
	void SetDownloadRequestHandler(MCBrowserDownloadRequestHandler *p_handler);
	void SetDownloadProgressHandler(MCBrowserDownloadProgressHandler *p_handler);

	MCBrowserEventHandler *GetEventHandler(void);
	MCBrowserJavaScriptHandler *GetJavaScriptHandler(void);
	MCBrowserProgressHandler *GetProgressHandler(void);
	MCBrowserFileDialogHandler *GetFileDialogHandler(void);
	MCBrowserDownloadRequestHandler *GetDownloadRequestHandler(void);
	MCBrowserDownloadProgressHandler *GetDownloadProgressHandler(void);

	virtual void OnNavigationBegin(bool p_in_frame, const char *p_url);
	virtual void OnNavigationComplete(bool p_in_frame, const char *p_url);
	virtual void OnNavigationFailed(bool p_in_frame, const char *p_url, const char *p_error);
	virtual void OnDocumentLoadBegin(bool p_in_frame, const char *p_url);
	virtual void OnDocumentLoadComplete(bool p_in_frame, const char *p_url);
	virtual void OnDocumentLoadFailed(bool p_in_frame, const char *p_url, const char *p_error);
	
	virtual void OnNavigationRequestUnhandled(bool p_in_frame, const char *p_url);
	
	virtual void OnJavaScriptCall(const char *p_handler, MCBrowserListRef p_params);

	virtual void OnProgressChanged(const char *p_url, uint32_t p_progress);

	virtual void FileDialogClearResponse(void);
	virtual bool FileDialogGetResponse(MCBrowserFileDialogResponse &r_response);
	virtual void FileDialogCancel(void);
	virtual void FileDialogSelectPaths(const char *p_paths, uindex_t p_selected_filter);

	virtual bool OnFileDialog(
		MCBrowserFileDialogType p_type,
		MCBrowserFileDialogOptions p_options,
		const char *p_title,
		const char *p_default_path,
		const char *p_filters,
		uindex_t p_default_filter
	);

	virtual void DownloadClearCancelled(void);
	virtual void DownloadSetCancelled(void);
	virtual bool DownloadGetCancelled(void);

	virtual void OnDownloadRequest(const char *p_url);
	virtual void OnDownloadProgress(const char *p_url, MCBrowserDownloadState p_state, uint32_t p_bytes_recieved, int32_t p_total_bytes);

	static bool BrowserListAdd(MCBrowser *p_browser);
	static void BrowserListRemove(MCBrowser *p_browser);
	static bool BrowserListIterate(MCBrowserIterateCallback p_callback, void *p_context);

private:
	struct MCBrowserListEntry
	{
		MCBrowser *browser;
		
		MCBrowserListEntry *next;
	};
	
	static MCBrowserListEntry *s_browser_list;
	
	MCBrowserEventHandler *m_event_handler;
	MCBrowserJavaScriptHandler *m_javascript_handler;
	MCBrowserProgressHandler *m_progress_handler;
	MCBrowserFileDialogHandler *m_file_dialog_handler;
	MCBrowserDownloadRequestHandler *m_download_request_handler;
	MCBrowserDownloadProgressHandler *m_download_progress_handler;

	bool m_file_dialog_have_response;
	MCBrowserFileDialogResponse m_file_dialog_response;

	bool m_download_cancelled;
};

////////////////////////////////////////////////////////////////////////////////

typedef bool (*MCBrowserFactoryCreationFunc)(MCBrowserFactoryRef &r_factory);
struct MCBrowserFactoryMap
{
	const char *factory_id;
	MCBrowserFactoryRef instance;
	
	MCBrowserFactoryCreationFunc constructor;
};

extern MCBrowserFactoryMap* s_factory_list;

////////////////////////////////////////////////////////////////////////////////

bool MCBrowserAddRunloopAction(MCBrowserRunloopCallback p_callback, void *p_context);
void MCBrowserRemoveRunloopAction(MCBrowserRunloopCallback p_callback, void *p_context);
bool MCBrowserRunloopWait();
void MCBrowserRunloopBreakWait();

bool MCBrowserMemoryAllocate(size_t p_size, void *&r_mem);
bool MCBrowserMemoryReallocate(void *p_mem, size_t p_new_size, void *& r_new_mem);
void MCBrowserMemoryDeallocate(void *p_mem);
void MCBrowserMemoryClear(void *p_mem, size_t p_size);

inline bool MCBrowserMemoryAllocateClear(size_t p_size, void *&r_mem)
{
	void *t_mem;
	if (!MCBrowserMemoryAllocate(p_size, t_mem))
		return false;
	MCBrowserMemoryClear(t_mem, p_size);
	r_mem = t_mem;
	return true;
}

template <class T>
inline bool MCBrowserMemoryNew(T*&r_obj)
{
	return MCBrowserMemoryAllocateClear(sizeof(T), (void*&)r_obj);
}

template <class T>
inline void MCBrowserMemoryDelete(T* p_obj)
{
	MCBrowserMemoryDeallocate(p_obj);
}

template<typename T>
inline bool MCBrowserMemoryReallocate(T *p_block, size_t p_new_size, T*& r_new_block)
{
	return MCBrowserMemoryReallocate((void*)p_block, p_new_size, (void*&)r_new_block);
}

template <class T>
inline bool MCBrowserMemoryNewArray(uindex_t p_count, T*&r_array)
{
	return MCBrowserMemoryAllocateClear(sizeof(T) * p_count, (void*&)r_array);
}

template <class T>
inline bool MCBrowserMemoryResizeArray(uindex_t p_new_count, T*& x_array, uindex_t& x_count)
{
	if (MCBrowserMemoryReallocate(x_array, p_new_count * sizeof(T), x_array))
	{
		if (p_new_count > x_count)
			MCBrowserMemoryClear((uint8_t *)(x_array) + x_count * sizeof(T), (p_new_count - x_count) * sizeof(T));
		x_count = p_new_count;
		return true;
	}
	return false;
}

template <class T>
inline void MCBrowserMemoryDeleteArray(T*p_array)
{
	MCBrowserMemoryDeallocate(p_array);
}
template <class T>
inline void MCBrowserMemoryClear(T &p_struct)
{
	MCBrowserMemoryClear(&p_struct, sizeof(T));
}

////////////////////////////////////////////////////////////////////////////////

struct MCBrowserValue
{
	MCBrowserValueType type;
	union
	{
		bool boolean;
		int32_t integer;
		double double_val;
		char *utf8_string;
		MCBrowserListRef array;
		MCBrowserDictionaryRef dictionary;
	};
};

void MCBrowserValueClear(MCBrowserValue &p_value);
bool MCBrowserValueCopy(const MCBrowserValue &p_src, MCBrowserValue &r_dst);

bool MCBrowserValueSetBoolean(MCBrowserValue &self, bool p_value);
bool MCBrowserValueGetBoolean(MCBrowserValue &self, bool &r_value);
bool MCBrowserValueSetInteger(MCBrowserValue &self, int32_t p_value);
bool MCBrowserValueGetInteger(MCBrowserValue &self, int32_t &r_value);
bool MCBrowserValueSetDouble(MCBrowserValue &self, double p_value);
bool MCBrowserValueGetDouble(MCBrowserValue &self, double &r_value);
bool MCBrowserValueSetUTF8String(MCBrowserValue &self, const char *p_value);
bool MCBrowserValueGetUTF8String(MCBrowserValue &self, char *&r_value);
bool MCBrowserValueSetList(MCBrowserValue &self, MCBrowserListRef p_value);
bool MCBrowserValueGetList(MCBrowserValue &self, MCBrowserListRef &r_value);
bool MCBrowserValueSetDictionary(MCBrowserValue &self, MCBrowserDictionaryRef p_value);
bool MCBrowserValueGetDictionary(MCBrowserValue &self, MCBrowserDictionaryRef &r_value);

bool MCBrowserListSetValue(MCBrowserListRef p_list, uint32_t p_index, const MCBrowserValue &p_value);
bool MCBrowserListGetValue(MCBrowserListRef p_list, uint32_t p_index, MCBrowserValue &r_value);
bool MCBrowserListAppendValue(MCBrowserListRef p_list, const MCBrowserValue &p_value);

bool MCBrowserDictionarySetValue(MCBrowserDictionaryRef p_dict, const char *p_key, const MCBrowserValue &p_value);
bool MCBrowserDictionaryGetValue(MCBrowserDictionaryRef p_dict, const char *p_key, MCBrowserValue &r_value);

////////////////////////////////////////////////////////////////////////////////

#endif //__LIBBROWSER_INTERNAL_H__
