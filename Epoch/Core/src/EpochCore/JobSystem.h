#pragma once
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <thread>
#include <future>

namespace Epoch::Core
{
	enum class JobPriority { High, Normal, Low };

	struct Job
	{
		std::function<void()> Task;
		std::string Name;
		JobPriority Priority = JobPriority::Normal;
		std::vector<std::shared_future<void>> Dependencies;
	};

	class JobSystem
	{
	public:
		//TEMP
		static JobSystem& GetJobSystem() { static JobSystem instance; return instance; }

		void Init();
		void ShutDown();

		template<typename F>
		auto SubmitJob(const std::string& name, F&& func, JobPriority priority = JobPriority::Normal, const std::vector<std::shared_future<void>>& deps = {})
		{
			using ReturnType = std::invoke_result_t<F>;

			auto task = std::make_shared<std::packaged_task<ReturnType()>>(
				[func = std::forward<F>(func), name]() {
					return func();
				});

			auto future = task->get_future().share();

			{
				std::unique_lock<std::mutex> lock(myMutex);
				Job job;
				job.Task = [task]() { (*task)(); };
				job.Name = name;
				job.Priority = priority;
				job.Dependencies = deps;

				GetQueue(priority).emplace_back(std::move(job));
			}
			myCondition.notify_one();
			return future;
		}

		bool AllTasksDone();
		void WaitUntilDone();

	private:
		void WorkerLoop();

		bool PopReadyJob(Job& outJob);
		bool TryPopReady(std::deque<Job>& aQueue, Job& outJob);

		std::deque<Job>& GetQueue(JobPriority aPriority);

	private:
		std::vector<std::thread> myWorkers;
		std::deque<Job> myHighPriorityJobs;
		std::deque<Job> myNormalPriorityJobs;
		std::deque<Job> myLowPriorityJobs;
		std::mutex myMutex;
		std::condition_variable myCondition;
		std::condition_variable myDoneCondition;
		std::atomic<bool> myRunning = false;
		std::atomic<uint32_t> myActiveTasks = 0;
		unsigned myWorkerCount = 0;
	};
}
