#include <vector>
#include <memory>
#include <qpl/qpl.h>

inline uint32_t perform_bitwise_and_sum(const uint8_t* source1, 
                                        const uint8_t* source2_mask,
                                        const uint8_t* out,
                                        uint32_t num_bits) {
    qpl_job *job = nullptr;
    uint32_t job_size = 0;

    qpl_path_t path = qpl_path_software;  // Use software path unless you verified hardware support

    if (qpl_get_job_size(path, &job_size) != QPL_STS_OK) {
        std::cerr << "Failed to get QPL job size.\n";
        return 0;
    }

    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    job = reinterpret_cast<qpl_job*>(job_buffer.get());

    if (qpl_init_job(path, job) != QPL_STS_OK) {
        std::cerr << "Failed to initialize QPL job.\n";
        return 0;
    }

    job->next_in_ptr        = const_cast<uint8_t*>(source1);
    job->available_in       = (num_bits + 7) / 8;
    job->next_src2_ptr      = const_cast<uint8_t*>(source2_mask);
    job->available_src2     = (num_bits + 7) / 8;
    job->src1_bit_width     = 1;
    job->src2_bit_width     = 1;
    job->num_input_elements = num_bits;
    job->op                 = qpl_op_select;
    job->out_bit_width      = qpl_ow_nom;
    job->next_out_ptr       = const_cast<uint8_t*>(out);
    job->available_out      = (num_bits + 7) / 8;

    qpl_status status = qpl_execute_job(job);
    uint32_t result_sum = 0;

    if (status == QPL_STS_OK) {
        result_sum = job->sum_value;
    } else {
        std::cerr << "QPL job execution failed: " << status << "\n";
    }

    qpl_fini_job(job);
    return result_sum;
}
