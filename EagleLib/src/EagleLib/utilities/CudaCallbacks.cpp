#include "CudaCallbacks.hpp"

using namespace EagleLib::cuda;

void EagleLib::cuda::ICallback::cb_func_async(int status, void* user_data)
{
	auto _start = clock();
	pplx::create_task([user_data]()->void
	{
		auto cb = static_cast<ICallback*>(user_data);
		auto start = clock();
		cb->run();
		BOOST_LOG_TRIVIAL(info) << "Callback execution time: " << clock() - start;
		delete cb;
	});
	BOOST_LOG_TRIVIAL(trace) << "Callback launch time: " << clock() - _start;
}
void EagleLib::cuda::ICallback::cb_func(int status, void* user_data)
{
	auto cb = static_cast<ICallback*>(user_data);
	auto start = clock();
	cb->run();
	BOOST_LOG_TRIVIAL(info) << "Callback execution time: " << clock() - start;
	delete cb;
}
ICallback::~ICallback()
{
}


scoped_event_timer::scoped_event_timer(cv::cuda::Stream& stream, const std::string& scope_name): _stream(stream), _scope_name(scope_name)
{
	start = clock();
}
struct scoped_event_timer_data
{
	std::string scope_name;
	clock_t start_time;
};
scoped_event_timer::~scoped_event_timer()
{
	/*_end.record(_stream);
	cv::cuda::Event end = _end;
	cv::cuda::Event start = _start;
	cv::cuda::Stream stream = _stream;
	std::string name = _scope_name;
	pplx::create_task(
		[start, end, stream, name]()->void
	{
		cv::cuda::Event ev = end; // end is capture as a const value, need to make a copy to get rid of const
		ev.waitForCompletion();
		BOOST_LOG_TRIVIAL(info) << "[" << name << "] ellapsed time " << 
			cv::cuda::Event::elapsedTime(start, end);
	});	*/
	scoped_event_timer_data data;
	data.scope_name = _scope_name;
	data.start_time = start;
	EagleLib::cuda::enqueue_callback<scoped_event_timer_data,void>(data, [] )
}