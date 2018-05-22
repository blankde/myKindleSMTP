struct Entry{
	const char* recipe;//calibre收据
	const char* seed_path;//rss种子路径
	const char* book;//生成图书名称
	const char* recver;//接收邮箱

	bool is_fetch;
};