//
// Created by lukashutak on 31.12.17.
//

#ifndef LIBFDS_TGENERATOR_H
#define LIBFDS_TGENERATOR_H

#include <cstdint>
#include <memory>

/**
 * \brief Simple generator of IPFIX templates
 *
 * This class allows to create also malformed templates for testing purposes
 */
class Generator {
private:

/** Default allocation size */
    static constexpr size_t DEF_SIZE = 20;
    /** Template buffer */
    std::unique_ptr<uint8_t[]> data;
    /** Allocated size of the buffer */
    size_t size_alloc;
    /** Used size of the buffer */
    size_t size_used;

    /** Number of fields in record **/
    uint16_t field_cnt;
public:
    /**
     * \brief Constructor
     * \param[in] id        Template ID
     * \param[in] scope_cnt Scope field count
     */
    Generator(uint16_t id, uint16_t scope_cnt = 0);

    Generator();
    /**
     * \brief Destructor
     */
    ~Generator() {};

    // Disable copy constructors
    Generator(Generator const &) = delete;
    Generator &operator=(Generator const &) = delete;
    // Define move operators
    Generator &operator=(Generator &&o);

    /**
     * \brief Append a new template field
     * \param[in] ie_id  Information Element ID
     * \param[in] len    Data length
     * \param[in] ie_en  Private Enterprise Number
     */
    void
    append(uint16_t ie_id, uint16_t len, uint32_t ie_en = 0);
    /**
     * \brief Get a pointer to the template
     * \return
     */
    const void *
    get();

    /**
     * \brief Get a length of the template
     * \return Length
     */
    size_t
    length();
};


#endif //LIBFDS_TGENERATOR_H
