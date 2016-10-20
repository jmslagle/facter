#include <internal/facts/freebsd/memory_resolver.hpp>
#include <leatherman/execution/execution.hpp>
#include <leatherman/logging/logging.hpp>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <sys/vmmeter.h>
#include <vm/vm_param.h>

using namespace std;
using namespace leatherman::execution;

namespace facter { namespace facts { namespace freebsd {

    memory_resolver::data memory_resolver::collect_data(collection& facts)
    {
        data result;

        // Get the system page size
        int page_size = 0;
	size_t page_len = sizeof(page_size);
	if (sysctlbyname("vm.stats.vm.v_page_size", &page_size, &page_len, nullptr, 0) == -1) {
		LOG_DEBUG("Unable to get system page size: {1} ({2})", strerror(errno), errno);
	} else {
		long totalmem = 0;
		size_t totalmem_len = sizeof(totalmem);
		if (sysctlbyname("hw.physmem", &totalmem, &totalmem_len, nullptr, 0) == -1) {
			LOG_DEBUG("Unable to get total memory: {1} ({2})", strerror(errno), errno);
		} else {
            		result.mem_total = static_cast<u_int64_t>(totalmem);
		}
		struct vmtotal vmtotal;
		size_t vmtotal_len = sizeof(vmtotal);
		if (sysctlbyname("vm.vmtotal", &vmtotal, &vmtotal_len, nullptr, 0) == -1) {
			LOG_DEBUG("Unable to get vmtotal: {1} ({2})", strerror(errno), errno);
		} else {
            		result.mem_free = static_cast<u_int64_t>(vmtotal.t_free) * page_size;
		}

        }
	long totalswap = 0;
	size_t totalswap_len = sizeof(totalswap);

	if (sysctlbyname("vm.swap_total", &totalswap, &totalswap_len, nullptr, 0) == -1) {
		LOG_DEBUG("Unable to get swaptotal: {1} ({2})", strerror(errno), errno);
	} else {
		result.swap_total = static_cast<u_int64_t>(totalswap);
	}
	// Loop through swap devs to get used
	long swap_used = 0;

	struct xswdev xsw;
	size_t mibsize;
	size_t xsw_len = sizeof(xsw);
	int mib[16], i; // Up to 16 swap devs

	mibsize = sizeof(mib)/sizeof(mib[0]);
	if (sysctlnametomib("vm.swap_info", mib, &mibsize) == -1) {
		LOG_DEBUG("Unable to get swap info: {1} ({2})", strerror(errno), errno);
	} else {
		for (i=0; i<16; i++) {
			mib[mibsize]=i; // Select the right MIB
			if (sysctl(mib, mibsize + 1, &xsw, &xsw_len, nullptr, 0) == -1) {
				break; // No more swap devs
			}
			swap_used = swap_used + xsw.xsw_used;
		}
		result.swap_free = static_cast<u_int64_t>(totalswap) - swap_used;
	}
	
        return result;
    }

}}}  // namespace facter::facts::freebsd
