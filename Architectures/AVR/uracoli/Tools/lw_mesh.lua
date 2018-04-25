-- do
    --
    -- uracoli Leightweight Mesh dissector for Wireshark
    -- To activate copy this file to Wiresharks plugin directory, which can
    -- be found in: Wireshark Menu Help --> About --> Folders
    --

    local lw_proto = Proto ("lw", "Lightweight Mesh protocoll")

    local ep_table = {
        [0x0] = "Command",
        [0x1] = "User Endpoint 1",
        [0x2] = "User Endpoint 2",
        [0x3] = "User Endpoint 3",
        [0x4] = "User Endpoint 4",
        [0x5] = "User Endpoint 5",
        [0x6] = "User Endpoint 6",
        [0x7] = "User Endpoint 7",
        [0x8] = "User Endpoint 8",
        [0x9] = "User Endpoint 9",
        [0xa] = "User Endpoint 10",
        [0xb] = "User Endpoint 11",
        [0xc] = "User Endpoint 12",
        [0xd] = "User Endpoint 13",
        [0xe] = "User Endpoint 14",
        [0xf] = "User Endpoint 15",
    }

    local cmd_id_table = {
        [0x00] = "LW_CMD_ACK",
        [0x01] = "LW_CMD_ROUTE_ERROR"
    }

    local fields = lw_proto.fields

    local f_fctl = ProtoField.uint8("lw.fctl", "Frame Control Field", base.HEX)
    fields.f_fctl_1 = ProtoField.uint8("lw.fctl.ack"
                                        , "Acknowledgement Request"
                                        , base.HEX
                                        , { [1] = "True", [0] = "False"}, 0x01)
    fields.f_fctl_2 = ProtoField.uint8("lw.fctl.sec"
                                        , "Security Enabled"
                                        , base.HEX
                                        , { [1] = "True", [0] = "False"}, 0x02)
    fields.f_fctl_3 = ProtoField.uint8("lw.fctl.loc"
                                        , "Link Local"
                                        , base.HEX
                                        , { [1] = "True", [0] = "False"}, 0x04)

    local f_seq = ProtoField.uint8("lw.seq_no", "Sequence Number")
    local f_src_addr = ProtoField.uint16("lw.src_addr"
                                        , "Source Address", base.HEX)
    local f_dst_addr = ProtoField.uint16("lw.dst_addr"
                                        , "Destination Address", base.HEX)

    local f_ep = ProtoField.uint8("lw.ep", "Endpoints", base.HEX)
    fields.f_ep_1 = ProtoField.uint8("lw.src_ep", "Source Endpoint"
                                        , nil, ep_table, 0xF0)
    fields.f_ep_2 = ProtoField.uint8("lw.dst_ep", "Destination Endpoint"
                                        , nil, ep_table, 0x0F)

    local f_rawdata = ProtoField.bytes("lw.rawdata", "Data", base.HEX)

    local f_cmdid = ProtoField.uint8("lw.cmd", "Command", base.HEX
                                        , cmd_id_table)
    -- LW_CMD_ACK
    local f_ack_seq = ProtoField.uint8("lw.ack_seq","Sequence Number")
    local f_ack_ctrl = ProtoField.uint8("lw.ack_ctrl", "Control Message"
                                        , base.HEX)
    -- LW_CMD_ROUTE_ERROR
    local f_rer_src_addr = ProtoField.uint16("lw.route_error_src"
                                        , "Source Address", base.HEX)
    local f_rer_dst_addr = ProtoField.uint16("lw.route_error_dst"
                                        , "Destination Address", base.HEX)


    -- Init function, called before any packet is dissected
    function lw_proto.init()
        -- print("lw.init")
    end

    function dissect_ack (offs, buffer, pinfo, tree)
        local st = tree --tree:add(p2p, buffer())
        st:add(f_ack_seq, buffer(offs, 1))
        st:add(f_ack_ctrl, buffer(offs+1, 1))
    end

    function dissect_route_error(offs, buffer, pinfo, tree)
        tree:add(f_rer_src_addr, buffer(offs, 2))
        tree:add(f_rer_dst_addr, buffer(offs+2, 2))
    end


    -- The main dissector function
    function lw_proto.dissector (buffer, pinfo, tree)
        local fcf = buffer(0,1):uint() + buffer(1,1):uint() * 256
        local frame_len = buffer:len()
        local mac_header_len = 9
        local lw_header_len = 7
        local mac_fcf_len = 2
        if (fcf == 0x8841 or fcf == 0x8861) then
            -- dissect header
            local proto_string = "Lightweight Mesh"
            local info_string = ""

            local lw_frame_len = frame_len - mac_header_len - mac_fcf_len
            local lw_frame = buffer(mac_header_len, lw_frame_len)

            local subtree = tree:add(lw_proto, lw_frame, "Lightweight Mesh")

            local fctl_sub = subtree:add_le(f_fctl, lw_frame(0, 1))
            fctl_sub:add_le(fields.f_fctl_1, lw_frame(0, 1))
            fctl_sub:add_le(fields.f_fctl_2, lw_frame(0, 1))
            fctl_sub:add_le(fields.f_fctl_3, lw_frame(0, 1))

            subtree:add_le(f_seq, lw_frame(1, 1))
            subtree:add_le(f_src_addr, lw_frame(2, 2))
            subtree:add_le(f_dst_addr, lw_frame(4, 2))

            local ep_sub = subtree:add_le(f_ep, lw_frame(6, 1))
            ep_sub:add_le(fields.f_ep_1, lw_frame(6, 1))
            ep_sub:add_le(fields.f_ep_2, lw_frame(6, 1))

            if(lw_frame(6, 1):uint() == 0) then
                -- dissect command frame
                local cmdid = lw_frame(lw_header_len, 1)
                local cmdname = cmd_id_table[cmdid:uint()]

                if(cmdname == "LW_CMD_ACK") then
                    info_string = "Ack Command"
                    local cmd_sub = subtree:add(lw_frame(lw_header_len
                                    , lw_frame_len - lw_header_len)
                                    , info_string)
                    cmd_sub:add_le(f_cmdid, cmdid)
                    dissect_ack(lw_header_len + 1, lw_frame, pinfo, cmd_sub)

                elseif(cmdname == "LW_CMD_ROUTE_ERROR") then
                    info_string = "Route Error Command"
                    local cmd_sub = subtree:add(lw_frame(lw_header_len
                                    , lw_frame_len - lw_header_len)
                                    , info_string)
                    cmd_sub:add_le(f_cmdid, cmdid)
                    dissect_ack(lw_header_len + 1, lw_frame, pinfo, cmd_sub)

                else
                    info_string = "Unknown Command"
                    local cmd_sub = subtree:add(lw_frame(lw_header_len
                                    , lw_frame_len - lw_header_len)
                                    , info_string)
                    cmd_sub:add_le(f_cmdid, cmdid)

                end
            else
                info_string = "Data"
                local data_sub = subtree:add(
                                    lw_frame(lw_header_len
                                    , lw_frame_len - lw_header_len)
                                    , "Data (" .. lw_frame_len - lw_header_len
                                        .. " bytes)")
                data_sub:add_le(f_rawdata, lw_frame(lw_header_len
                                            , lw_frame_len - lw_header_len))
            end
            pinfo.cols.info = info_string
            pinfo.cols.protocol = proto_string
        end
    end

    -- Create the protocol fields
    lw_proto.fields = { -- Lightweight Mesh frame header and payload
                   f_fctl, f_seq, f_src_addr, f_dst_addr,
                   f_ep, f_rawdata, f_cmdid,
                   -- LW_CMD_ACK
                   f_ack_seq, f_ack_ctrl,
                   -- LW_CMD_ROUTE_ERROR
                   f_rer_src_addr, f_rer_dst_addr,
                 }

     -- Register dissector
    register_postdissector (lw_proto)

-- end
