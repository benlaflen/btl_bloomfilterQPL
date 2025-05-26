#include <vector>
#include <memory>
#include "qpl/qpl.h"

inline uint32_t perform_bitwise_and_sum(const uint8_t* source1, 
                                 const uint8_t* source2_mask,
                                 uint32_t num_bits) {
    qpl_job *job = nullptr;
    uint32_t job_size = 0;

    // Initialize QPL job
    qpl_get_job_size(qpl_path_hardware, &job_size);
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    job = reinterpret_cast<qpl_job*>(job_buffer.get());
    qpl_init_job(qpl_path_hardware, job);

    // Setup the job parameters for select operation
    job->next_in_ptr        = const_cast<uint8_t*>(source1);
    job->available_in       = (num_bits + 7) / 8;   // bytes
    job->next_src2_ptr      = const_cast<uint8_t*>(source2_mask);
    job->available_src2     = (num_bits + 7) / 8;   // bytes
    job->src1_bit_width     = 1;
    job->src2_bit_width     = 1;
    job->num_input_elements = num_bits;
    job->op                 = qpl_op_select;
    job->out_bit_width      = qpl_ow_nom;

    // Skip output buffer since we only need sum_value
    job->next_out_ptr       = nullptr;
    job->available_out      = 0;

    // Execute job
    qpl_status status = qpl_execute_job(job);

    uint32_t result_sum = 0;
    if (status == QPL_STS_OK) {
        result_sum = job->sum_value;
    } else {
        // Handle error as needed
    }

    qpl_fini_job(job);

    return result_sum;
}
