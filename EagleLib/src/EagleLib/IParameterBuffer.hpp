#pragma once

#include <parameters/LokiTypeInfo.h>
#include <boost/any.hpp>
#include <signals/logging.hpp>

namespace EagleLib
{
    class IParameterBuffer
    {
    public:
        virtual void SetBufferSize(int size) = 0;
        virtual boost::any& GetParameter(Loki::TypeInfo, const std::string& name, int frameNumber) = 0;

        template<typename T> bool GetParameter(T& param, const std::string& name, int frameNumber)
        {
            auto& parameter = GetParameter(Loki::TypeInfo(typeid(T)), name, frameNumber);
            if (parameter.empty())
                return false;
            try
            {
                param = boost::any_cast<T>(parameter);
                return true;
            }
            catch (boost::bad_any_cast& bad_cast)
            {
                LOG(trace) << bad_cast.what();
            }
            return false;
        }
        template<typename T> bool SetParameter(T& param, const std::string& name, int frameNumber)
        {
            auto& parameter = GetParameter(Loki::TypeInfo(typeid(T)), name, frameNumber);
            parameter = param;
            return true;
        }
    };

}