#pragma once

#include <cstddef>
#include <cstdint>

namespace solar {

struct TelemetryHistorySettings final {
    static constexpr std::uint32_t DefaultSampleIntervalSeconds = 10;
    static constexpr std::uint32_t DefaultBatchIntervalSeconds = 300;
    static constexpr std::uint32_t MinimumSampleIntervalSeconds = 1;
    static constexpr std::uint32_t MaximumSampleIntervalSeconds = 3600;
    static constexpr std::uint32_t MinimumBatchIntervalSeconds = 1;
    static constexpr std::uint32_t MaximumBatchIntervalSeconds = 7 * 24 * 60 * 60;

    // Timestamp (int64) plus four double values. SQLite row/page overhead is
    // deliberately excluded; this is the RAM payload estimate shown in HMI.
    static constexpr std::size_t EstimatedSampleBytes = 8 + (4 * 8);

    std::uint32_t sampleIntervalSeconds{DefaultSampleIntervalSeconds};
    std::uint32_t batchIntervalSeconds{DefaultBatchIntervalSeconds};

    [[nodiscard]] bool valid() const noexcept {
        return sampleIntervalSeconds >= MinimumSampleIntervalSeconds
            && sampleIntervalSeconds <= MaximumSampleIntervalSeconds
            && batchIntervalSeconds >= MinimumBatchIntervalSeconds
            && batchIntervalSeconds <= MaximumBatchIntervalSeconds
            && batchIntervalSeconds >= sampleIntervalSeconds;
    }

    [[nodiscard]] std::size_t samplesPerBatch() const noexcept {
        if (sampleIntervalSeconds == 0) {
            return 0;
        }
        return (static_cast<std::size_t>(batchIntervalSeconds)
                + sampleIntervalSeconds - 1)
            / sampleIntervalSeconds;
    }

    [[nodiscard]] std::size_t estimatedBufferBytes() const noexcept {
        return samplesPerBatch() * EstimatedSampleBytes;
    }

    friend bool operator==(
        const TelemetryHistorySettings& left,
        const TelemetryHistorySettings& right) noexcept = default;
};

} // namespace solar
