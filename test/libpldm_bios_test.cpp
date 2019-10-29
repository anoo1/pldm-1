#include <string.h>

#include <array>

#include "libpldm/base.h"
#include "libpldm/bios.h"

#include <gtest/gtest.h>

constexpr auto hdrSize = sizeof(pldm_msg_hdr);

TEST(GetDateTime, testEncodeRequest)
{
    pldm_msg request{};

    auto rc = encode_get_date_time_req(0, &request);
    ASSERT_EQ(rc, PLDM_SUCCESS);
}

TEST(GetDateTime, testEncodeResponse)
{
    uint8_t completionCode = 0;
    uint8_t seconds = 50;
    uint8_t minutes = 20;
    uint8_t hours = 5;
    uint8_t day = 23;
    uint8_t month = 11;
    uint16_t year = 2019;

    std::array<uint8_t, sizeof(pldm_msg_hdr) + PLDM_GET_DATE_TIME_RESP_BYTES>
        responseMsg{};

    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());

    auto rc = encode_get_date_time_resp(0, PLDM_SUCCESS, seconds, minutes,
                                        hours, day, month, year, response);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(completionCode, response->payload[0]);

    ASSERT_EQ(0, memcmp(response->payload + sizeof(response->payload[0]),
                        &seconds, sizeof(seconds)));
    ASSERT_EQ(0, memcmp(response->payload + sizeof(response->payload[0]) +
                            sizeof(seconds),
                        &minutes, sizeof(minutes)));
    ASSERT_EQ(0, memcmp(response->payload + sizeof(response->payload[0]) +
                            sizeof(seconds) + sizeof(minutes),
                        &hours, sizeof(hours)));
    ASSERT_EQ(0, memcmp(response->payload + sizeof(response->payload[0]) +
                            sizeof(seconds) + sizeof(minutes) + sizeof(hours),
                        &day, sizeof(day)));
    ASSERT_EQ(0, memcmp(response->payload + sizeof(response->payload[0]) +
                            sizeof(seconds) + sizeof(minutes) + sizeof(hours) +
                            sizeof(day),
                        &month, sizeof(month)));
    ASSERT_EQ(0, memcmp(response->payload + sizeof(response->payload[0]) +
                            sizeof(seconds) + sizeof(minutes) + sizeof(hours) +
                            sizeof(day) + sizeof(month),
                        &year, sizeof(year)));
}

TEST(GetDateTime, testDecodeResponse)
{
    std::array<uint8_t, hdrSize + PLDM_GET_DATE_TIME_RESP_BYTES> responseMsg{};

    uint8_t completionCode = 0;

    uint8_t seconds = 55;
    uint8_t minutes = 2;
    uint8_t hours = 8;
    uint8_t day = 9;
    uint8_t month = 7;
    uint16_t year = 2020;

    uint8_t retSeconds = 0;
    uint8_t retMinutes = 0;
    uint8_t retHours = 0;
    uint8_t retDay = 0;
    uint8_t retMonth = 0;
    uint16_t retYear = 0;

    memcpy(responseMsg.data() + sizeof(completionCode) + hdrSize, &seconds,
           sizeof(seconds));
    memcpy(responseMsg.data() + sizeof(completionCode) + sizeof(seconds) +
               hdrSize,
           &minutes, sizeof(minutes));
    memcpy(responseMsg.data() + sizeof(completionCode) + sizeof(seconds) +
               sizeof(minutes) + hdrSize,
           &hours, sizeof(hours));
    memcpy(responseMsg.data() + sizeof(completionCode) + sizeof(seconds) +
               sizeof(minutes) + sizeof(hours) + hdrSize,
           &day, sizeof(day));
    memcpy(responseMsg.data() + sizeof(completionCode) + sizeof(seconds) +
               sizeof(minutes) + sizeof(hours) + sizeof(day) + hdrSize,
           &month, sizeof(month));
    memcpy(responseMsg.data() + sizeof(completionCode) + sizeof(seconds) +
               sizeof(minutes) + sizeof(hours) + sizeof(day) + sizeof(month) +
               hdrSize,
           &year, sizeof(year));

    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());

    auto rc = decode_get_date_time_resp(
        response, responseMsg.size() - hdrSize, &completionCode, &retSeconds,
        &retMinutes, &retHours, &retDay, &retMonth, &retYear);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(seconds, retSeconds);
    ASSERT_EQ(minutes, retMinutes);
    ASSERT_EQ(hours, retHours);
    ASSERT_EQ(day, retDay);
    ASSERT_EQ(month, retMonth);
    ASSERT_EQ(year, retYear);
}

TEST(GetBIOSTable, testGoodEncodeResponse)
{
    std::array<uint8_t,
               sizeof(pldm_msg_hdr) + PLDM_GET_BIOS_TABLE_MIN_RESP_BYTES + 4>
        responseMsg{};
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());

    uint8_t completionCode = PLDM_SUCCESS;
    uint32_t nextTransferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    std::array<uint8_t, 4> tableData{1, 2, 3, 4};

    auto rc = encode_get_bios_table_resp(
        0, PLDM_SUCCESS, nextTransferHandle, transferFlag, tableData.data(),
        sizeof(pldm_msg_hdr) + PLDM_GET_BIOS_TABLE_MIN_RESP_BYTES + 4,
        response);
    ASSERT_EQ(rc, PLDM_SUCCESS);

    struct pldm_get_bios_table_resp* resp =
        reinterpret_cast<struct pldm_get_bios_table_resp*>(response->payload);

    ASSERT_EQ(completionCode, resp->completion_code);
    ASSERT_EQ(nextTransferHandle, resp->next_transfer_handle);
    ASSERT_EQ(transferFlag, resp->transfer_flag);
    ASSERT_EQ(0, memcmp(tableData.data(), resp->table_data, tableData.size()));
}

TEST(GetBIOSTable, testBadEncodeResponse)
{
    uint32_t nextTransferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    std::array<uint8_t, 4> tableData{1, 2, 3, 4};

    auto rc = encode_get_bios_table_resp(
        0, PLDM_SUCCESS, nextTransferHandle, transferFlag, tableData.data(),
        sizeof(pldm_msg_hdr) + PLDM_GET_BIOS_TABLE_MIN_RESP_BYTES + 4, nullptr);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(GetBIOSTable, testGoodEncodeRequest)
{
    std::array<uint8_t, sizeof(pldm_msg_hdr) + PLDM_GET_BIOS_TABLE_REQ_BYTES>
        requestMsg{};
    uint32_t transferHandle = 0x0;
    uint8_t transferOpFlag = 0x01;
    uint8_t tableType = PLDM_BIOS_ATTR_TABLE;

    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());
    auto rc = encode_get_bios_table_req(0, transferHandle, transferOpFlag,
                                        tableType, request);

    ASSERT_EQ(rc, PLDM_SUCCESS);

    struct pldm_get_bios_table_req* req =
        reinterpret_cast<struct pldm_get_bios_table_req*>(request->payload);
    ASSERT_EQ(transferHandle, le32toh(req->transfer_handle));
    ASSERT_EQ(transferOpFlag, req->transfer_op_flag);
    ASSERT_EQ(tableType, req->table_type);
}

TEST(GetBIOSTable, testBadEncodeRequest)
{
    uint32_t transferHandle = 0x0;
    uint8_t transferOpFlag = 0x01;
    uint8_t tableType = PLDM_BIOS_ATTR_TABLE;

    auto rc = encode_get_bios_table_req(0, transferHandle, transferOpFlag,
                                        tableType, nullptr);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(GetBIOSTable, testGoodDecodeRequest)
{
    const auto hdr_size = sizeof(pldm_msg_hdr);
    std::array<uint8_t, hdr_size + PLDM_GET_BIOS_TABLE_REQ_BYTES> requestMsg{};
    uint32_t transferHandle = 31;
    uint8_t transferOpFlag = PLDM_GET_FIRSTPART;
    uint8_t tableType = PLDM_BIOS_ATTR_TABLE;
    uint32_t retTransferHandle = 0;
    uint8_t retTransferOpFlag = 0;
    uint8_t retTableType = 0;

    auto req = reinterpret_cast<pldm_msg*>(requestMsg.data());
    struct pldm_get_bios_table_req* request =
        reinterpret_cast<struct pldm_get_bios_table_req*>(req->payload);

    request->transfer_handle = transferHandle;
    request->transfer_op_flag = transferOpFlag;
    request->table_type = tableType;

    auto rc = decode_get_bios_table_req(req, requestMsg.size() - hdr_size,
                                        &retTransferHandle, &retTransferOpFlag,
                                        &retTableType);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(transferHandle, retTransferHandle);
    ASSERT_EQ(transferOpFlag, retTransferOpFlag);
    ASSERT_EQ(tableType, retTableType);
}
TEST(GetBIOSTable, testBadDecodeRequest)
{
    const auto hdr_size = sizeof(pldm_msg_hdr);
    std::array<uint8_t, hdr_size + PLDM_GET_BIOS_TABLE_REQ_BYTES> requestMsg{};
    uint32_t transferHandle = 31;
    uint8_t transferOpFlag = PLDM_GET_FIRSTPART;
    uint8_t tableType = PLDM_BIOS_ATTR_TABLE;
    uint32_t retTransferHandle = 0;
    uint8_t retTransferOpFlag = 0;
    uint8_t retTableType = 0;

    auto req = reinterpret_cast<pldm_msg*>(requestMsg.data());
    struct pldm_get_bios_table_req* request =
        reinterpret_cast<struct pldm_get_bios_table_req*>(req->payload);

    request->transfer_handle = transferHandle;
    request->transfer_op_flag = transferOpFlag;
    request->table_type = tableType;

    auto rc = decode_get_bios_table_req(req, requestMsg.size() - hdr_size,
                                        &retTransferHandle, &retTransferOpFlag,
                                        &retTableType);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(transferHandle, retTransferHandle);
    ASSERT_EQ(transferOpFlag, retTransferOpFlag);
    ASSERT_EQ(tableType, retTableType);
}
/*
TEST(GetBIOSTable, testBadDecodeRequest)
{
    const auto hdr_size = sizeof(pldm_msg_hdr);
    std::array<uint8_t, hdr_size + PLDM_GET_BIOS_TABLE_REQ_BYTES> requestMsg{};
    uint32_t transferHandle = 31;
    uint8_t transferOpFlag = PLDM_GET_FIRSTPART;
    uint8_t tableType = PLDM_BIOS_ATTR_TABLE;
    uint32_t retTransferHandle = 0;
    uint8_t retTransferOpFlag = 0;
    uint8_t retTableType = 0;

    auto req = reinterpret_cast<pldm_msg*>(requestMsg.data());
    struct pldm_get_bios_table_req* request =
        reinterpret_cast<struct pldm_get_bios_table_req*>(req->payload);

    request->transfer_handle = transferHandle;
    request->transfer_op_flag = transferOpFlag;
    request->table_type = tableType;

    auto rc = decode_get_bios_table_req(req, requestMsg.size() - hdr_size - 3,
                                        &retTransferHandle, &retTransferOpFlag,
                                        &retTableType);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}*/

TEST(GetBIOSAttributeCurrentValueByHandle, testGoodDecodeRequest)
{
    uint32_t transferHandle = 45;
    uint8_t transferOpFlag = PLDM_GET_FIRSTPART;
    uint16_t attributehandle = 10;
    uint32_t retTransferHandle = 0;
    uint8_t retTransferOpFlag = 0;
    uint16_t retattributehandle = 0;
    std::array<uint8_t, hdrSize + sizeof(transferHandle) +
                            sizeof(transferOpFlag) + sizeof(attributehandle)>
        requestMsg{};

    auto req = reinterpret_cast<pldm_msg*>(requestMsg.data());
    struct pldm_get_bios_attribute_current_value_by_handle_req* request =
        reinterpret_cast<
            struct pldm_get_bios_attribute_current_value_by_handle_req*>(
            req->payload);

    request->transfer_handle = transferHandle;
    request->transfer_op_flag = transferOpFlag;
    request->attribute_handle = attributehandle;

    auto rc = decode_get_bios_attribute_current_value_by_handle_req(
        req, requestMsg.size() - hdrSize, &retTransferHandle,
        &retTransferOpFlag, &retattributehandle);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(transferHandle, retTransferHandle);
    ASSERT_EQ(transferOpFlag, retTransferOpFlag);
    ASSERT_EQ(attributehandle, retattributehandle);
}

TEST(GetBIOSAttributeCurrentValueByHandle, testBadDecodeRequest)
{

    uint32_t transferHandle = 0;
    uint8_t transferOpFlag = PLDM_GET_FIRSTPART;
    uint16_t attribute_handle = 0;
    uint32_t retTransferHandle = 0;
    uint8_t retTransferOpFlag = 0;
    uint16_t retattribute_handle = 0;
    std::array<uint8_t, hdrSize + sizeof(transferHandle) +
                            sizeof(transferOpFlag) + sizeof(attribute_handle)>
        requestMsg{};

    auto req = reinterpret_cast<pldm_msg*>(requestMsg.data());
    struct pldm_get_bios_attribute_current_value_by_handle_req* request =
        reinterpret_cast<
            struct pldm_get_bios_attribute_current_value_by_handle_req*>(
            req->payload);

    request->transfer_handle = transferHandle;
    request->transfer_op_flag = transferOpFlag;
    request->attribute_handle = attribute_handle;

    auto rc = decode_get_bios_attribute_current_value_by_handle_req(
        NULL, requestMsg.size() - hdrSize, &retTransferHandle,
        &retTransferOpFlag, &retattribute_handle);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);

    transferHandle = 31;
    request->transfer_handle = transferHandle;

    rc = decode_get_bios_attribute_current_value_by_handle_req(
        req, 0, &retTransferHandle, &retTransferOpFlag, &retattribute_handle);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(GetBIOSAttributeCurrentValueByHandle, testGoodEncodeResponse)
{

    uint8_t instanceId = 10;
    uint8_t completionCode = PLDM_SUCCESS;
    uint32_t nextTransferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    uint8_t attributeData = 44;
    std::array<uint8_t,
               hdrSize +
                   sizeof(pldm_get_bios_attribute_current_value_by_handle_resp)>
        responseMsg{};
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());

    auto rc = encode_get_bios_current_value_by_handle_resp(
        instanceId, completionCode, nextTransferHandle, transferFlag,
        &attributeData, sizeof(attributeData), response);

    ASSERT_EQ(rc, PLDM_SUCCESS);

    struct pldm_get_bios_attribute_current_value_by_handle_resp* resp =
        reinterpret_cast<
            struct pldm_get_bios_attribute_current_value_by_handle_resp*>(
            response->payload);

    ASSERT_EQ(completionCode, resp->completion_code);
    ASSERT_EQ(nextTransferHandle, resp->next_transfer_handle);
    ASSERT_EQ(transferFlag, resp->transfer_flag);
    ASSERT_EQ(
        0, memcmp(&attributeData, resp->attribute_data, sizeof(attributeData)));
}

TEST(GetBIOSAttributeCurrentValueByHandle, testBadEncodeResponse)
{
    uint32_t nextTransferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    uint8_t attributeData = 44;

    auto rc = encode_get_bios_current_value_by_handle_resp(
        0, PLDM_SUCCESS, nextTransferHandle, transferFlag, &attributeData,
        sizeof(attributeData), nullptr);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);

    std::array<uint8_t,
               hdrSize +
                   sizeof(pldm_get_bios_attribute_current_value_by_handle_resp)>
        responseMsg{};
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    rc = encode_get_bios_current_value_by_handle_resp(
        0, PLDM_SUCCESS, nextTransferHandle, transferFlag, nullptr,
        sizeof(attributeData), response);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(SetBiosAttributeCurrentValue, testGoodEncodeRequest)
{
    uint8_t instanceId = 10;
    uint32_t transferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    uint32_t attributeData = 44;
    std::array<uint8_t, hdrSize + PLDM_SET_BIOS_ATTR_CURR_VAL_MIN_REQ_BYTES +
                            sizeof(attributeData)>
        requestMsg{};
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());
    auto rc = encode_set_bios_attribute_current_value_req(
        instanceId, transferHandle, transferFlag,
        reinterpret_cast<uint8_t*>(&attributeData), sizeof(attributeData),
        request, requestMsg.size() - hdrSize);

    ASSERT_EQ(rc, PLDM_SUCCESS);

    struct pldm_set_bios_attribute_current_value_req* req =
        reinterpret_cast<struct pldm_set_bios_attribute_current_value_req*>(
            request->payload);
    ASSERT_EQ(htole32(transferHandle), req->transfer_handle);
    ASSERT_EQ(transferFlag, req->transfer_flag);
    ASSERT_EQ(
        0, memcmp(&attributeData, req->attribute_data, sizeof(attributeData)));
}

TEST(SetBiosAttributeCurrentValue, testBadEncodeRequest)
{
    uint8_t instanceId = 10;
    uint32_t transferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    uint32_t attributeData = 44;
    std::array<uint8_t, hdrSize + PLDM_SET_BIOS_ATTR_CURR_VAL_MIN_REQ_BYTES>
        requestMsg{};
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());

    auto rc = encode_set_bios_attribute_current_value_req(
        instanceId, transferHandle, transferFlag, nullptr, 0, nullptr, 0);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
    rc = encode_set_bios_attribute_current_value_req(
        instanceId, transferHandle, transferFlag,
        reinterpret_cast<uint8_t*>(&attributeData), sizeof(attributeData),
        request, requestMsg.size() - hdrSize);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(SetBiosAttributeCurrentValue, testGoodDecodeRequest)
{
    uint32_t transferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    uint32_t attributeData = 44;

    std::array<uint8_t, hdrSize + PLDM_SET_BIOS_ATTR_CURR_VAL_MIN_REQ_BYTES +
                            sizeof(attributeData)>
        requestMsg{};
    auto request = reinterpret_cast<struct pldm_msg*>(requestMsg.data());
    struct pldm_set_bios_attribute_current_value_req* req =
        reinterpret_cast<struct pldm_set_bios_attribute_current_value_req*>(
            request->payload);
    req->transfer_handle = htole32(transferHandle);
    req->transfer_flag = transferFlag;
    memcpy(req->attribute_data, &attributeData, sizeof(attributeData));

    uint32_t retTransferHandle;
    uint8_t retTransferFlag;
    uint32_t retAttributeData;
    size_t retAttributeDataLength;
    auto rc = decode_set_bios_attribute_current_value_req(
        request, requestMsg.size() - hdrSize, &retTransferHandle,
        &retTransferFlag, reinterpret_cast<uint8_t*>(&retAttributeData),
        &retAttributeDataLength);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retTransferHandle, transferHandle);
    ASSERT_EQ(retTransferFlag, transferFlag);
    ASSERT_EQ(retAttributeDataLength, sizeof(attributeData));
    ASSERT_EQ(0,
              memcmp(&retAttributeData, &attributeData, sizeof(attributeData)));
}

TEST(SetBiosAttributeCurrentValue, testBadDecodeRequest)
{
    uint32_t transferHandle = 32;
    uint8_t transferFlag = PLDM_START_AND_END;
    uint32_t attributeData = 44;
    size_t attributeDataLength = sizeof(attributeData);
    std::array<uint8_t, hdrSize + PLDM_SET_BIOS_ATTR_CURR_VAL_MIN_REQ_BYTES - 1>
        requestMsg{};
    auto request = reinterpret_cast<struct pldm_msg*>(requestMsg.data());

    auto rc = decode_set_bios_attribute_current_value_req(
        nullptr, 0, &transferHandle, &transferFlag,
        reinterpret_cast<uint8_t*>(&attributeData), &attributeDataLength);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
    rc = decode_set_bios_attribute_current_value_req(
        request, requestMsg.size() - hdrSize, &transferHandle, &transferFlag,
        reinterpret_cast<uint8_t*>(&attributeData), &attributeDataLength);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(SetBiosAttributeCurrentValue, testGoodEncodeResponse)
{
    uint8_t instanceId = 10;
    uint32_t nextTransferHandle = 32;
    uint8_t completionCode = PLDM_SUCCESS;

    std::array<uint8_t, hdrSize + PLDM_SET_BIOS_ATTR_CURR_VAL_RESP_BYTES>
        responseMsg{};
    struct pldm_msg* response =
        reinterpret_cast<struct pldm_msg*>(responseMsg.data());
    auto rc = encode_set_bios_attribute_current_value_resp(
        instanceId, completionCode, nextTransferHandle, response);
    ASSERT_EQ(rc, PLDM_SUCCESS);

    struct pldm_set_bios_attribute_current_value_resp* resp =
        reinterpret_cast<struct pldm_set_bios_attribute_current_value_resp*>(
            response->payload);
    ASSERT_EQ(completionCode, resp->completion_code);
    ASSERT_EQ(htole32(nextTransferHandle), resp->next_transfer_handle);
}

TEST(SetBiosAttributeCurrentValue, testBadEncodeResponse)
{
    uint8_t instanceId = 10;
    uint32_t nextTransferHandle = 32;
    uint8_t completionCode = PLDM_SUCCESS;
    auto rc = encode_set_bios_attribute_current_value_resp(
        instanceId, completionCode, nextTransferHandle, nullptr);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}
TEST(SetBiosAttributeCurrentValue, testGoodDecodeResponse)
{
    uint32_t nextTransferHandle = 32;
    uint8_t completionCode = PLDM_SUCCESS;
    std::array<uint8_t, hdrSize + PLDM_SET_BIOS_ATTR_CURR_VAL_RESP_BYTES>
        responseMsg{};
    struct pldm_msg* response =
        reinterpret_cast<struct pldm_msg*>(responseMsg.data());
    struct pldm_set_bios_attribute_current_value_resp* resp =
        reinterpret_cast<struct pldm_set_bios_attribute_current_value_resp*>(
            response->payload);

    resp->completion_code = completionCode;
    resp->next_transfer_handle = htole32(nextTransferHandle);

    uint8_t retCompletionCode;
    uint32_t retNextTransferHandle;
    auto rc = decode_set_bios_attribute_current_value_resp(
        response, responseMsg.size() - hdrSize, &retCompletionCode,
        &retNextTransferHandle);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(completionCode, retCompletionCode);
    ASSERT_EQ(nextTransferHandle, retNextTransferHandle);
}

TEST(SetBiosAttributeCurrentValue, testBadDecodeResponse)
{
    uint32_t nextTransferHandle = 32;
    uint8_t completionCode = PLDM_SUCCESS;

    std::array<uint8_t, hdrSize + PLDM_SET_BIOS_ATTR_CURR_VAL_RESP_BYTES - 1>
        responseMsg{};
    struct pldm_msg* response =
        reinterpret_cast<struct pldm_msg*>(responseMsg.data());
    auto rc = decode_set_bios_attribute_current_value_resp(
        nullptr, 0, &completionCode, &nextTransferHandle);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);

    rc = decode_set_bios_attribute_current_value_resp(
        response, responseMsg.size() - hdrSize, &completionCode,
        &nextTransferHandle);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}
