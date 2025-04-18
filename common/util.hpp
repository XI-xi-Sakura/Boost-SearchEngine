#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include "log.hpp"
#include "/home/ubuntu/Boost/cppjieba/include/cppjieba/Jieba.hpp" //

namespace ns_util
{
    class FileUtil
    {
    public:
        static bool ReadFile(const std::string &file_path, std::string *out)
        {
            std::ifstream in(file_path, std::ios::in);
            if (!in.is_open())
            {
                std::cerr << "open file " << file_path << " error" << std::endl;
                return false;
            }

            std::string line;
            // 如何理解getline读取到文件结束呢？？getline的返回值是一个&，while(bool), 本质是因为重载了强制类型转化
            while (std::getline(in, line))
            {
                *out += line;
            }

            in.close();
            return true;
        }
    };
    class StringUtil
    {
    public:
        //  template<typename SequenceSequenceT, typename RangeT, typename PredicateT>
        //  SequenceSequenceT& split(
        //      SequenceSequenceT& Result,  //输出参数，用于存储分割后的子字符串的容器，通常是 std::vector<std::string> 类型。
        //
        //      RangeT& Input,              //输入参数，要分割的字符串。
        //
        //      PredicateT Pred,            //谓词函数，用于指定分割的依据。常见的有 boost::is_any_of，
        //                                  //它会将输入字符串中出现指定字符集中任意字符的位置作为分割点。
        //
        //      token_compress_mode_type eCompress = token_compress_off
        //                                  // 分割模式标志，有两个可选值：
        //                                  // boost::token_compress_off：默认值，连续的分隔符会被当作多个分隔符处理，可能会产生空的子字符串。
        //                                  // boost::token_compress_on：连续的分隔符会被压缩成一个分隔符处理，不会产生空的子字符串。
        //  );

        static void Split(const std::string &target, std::vector<std::string> *out, const std::string &sep)
        {
            // boost split
            boost::split(*out, target, boost::is_any_of(sep), boost::token_compress_on);
        }
    };

    const char *const DICT_PATH = "../cppjieba/dict/jieba.dict.utf8";
    const char *const HMM_PATH = "../cppjieba/dict/hmm_model.utf8";
    const char *const USER_DICT_PATH = "../cppjieba/dict/user.dict.utf8";
    const char *const IDF_PATH = "../cppjieba/dict/idf.utf8";
    const char *const STOP_WORD_PATH = "../cppjieba/dict/stop_words.utf8";

    class JiebaUtil
    {
    private:
        // static cppjieba::Jieba jieba;
        cppjieba::Jieba jieba;
        std::unordered_map<std::string, bool> stop_words;

    private:
        JiebaUtil() : jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH)
        {
        }
        JiebaUtil(const JiebaUtil &) = delete;

        static JiebaUtil *instance;

    public:
        static JiebaUtil *get_instance()
        {
            static std::mutex mtx;
            if (nullptr == instance)
            {
                mtx.lock();
                if (nullptr == instance)
                {
                    instance = new JiebaUtil();
                    instance->InitJiebaUtil();
                }
                mtx.unlock();
            }

            return instance;
        }

        void InitJiebaUtil()
        {
            std::ifstream in(STOP_WORD_PATH);
            if (!in.is_open())
            {
                LOG(FATAL, "load stop words file error");
                return;
            }

            std::string line;
            while (std::getline(in, line))
            {
                stop_words.insert({line, true});
            }

            in.close();
        }

        void CutStringHelper(const std::string &src, std::vector<std::string> *out)
        {
            jieba.CutForSearch(src, *out);
            for (auto iter = out->begin(); iter != out->end();)
            {
                auto it = stop_words.find(*iter);
                if (it != stop_words.end())
                {
                    // 说明当前的string 是暂停词，需要去掉
                    iter = out->erase(iter);
                }
                else
                {
                    iter++;
                }
            }
        }

    public:
        static void CutString(const std::string &src, std::vector<std::string> *out)
        {
            ns_util::JiebaUtil::get_instance()->CutStringHelper(src, out);
            // jieba.CutForSearch(src, *out);
        }
    };

    JiebaUtil *JiebaUtil::instance = nullptr;
    // cppjieba::Jieba JiebaUtil::jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
}