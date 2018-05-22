和Huginn，calibre结合使用爬取微信公众号并投递到kindle的C++程序



### 代码目录

Scenarios：Huginn Scenarios

src：源代码目录

exe：可执行文件和相关配置文件



### 配置文件

entry.json 投递列表

path.conf calibre抓取rss写入的文件文章

sig.txt 每个rss seed的MD5值，当该值不变是不发生投递

log.txt 投递日志

feedlog.txt 抓取rss日志

.recipe calibre的recipe文件