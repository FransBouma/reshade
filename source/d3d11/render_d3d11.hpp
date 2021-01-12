/*
 * Copyright (C) 2021 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#pragma once

#include "com_ptr.hpp"
#include "com_tracking.hpp"
#include "addon_manager.hpp"
#include <d3d11_4.h>

namespace reshade::d3d11
{
	void convert_resource_desc(const api::resource_desc &desc, D3D11_BUFFER_DESC &internal_desc);
	void convert_resource_desc(const api::resource_desc &desc, D3D11_TEXTURE1D_DESC &internal_desc);
	void convert_resource_desc(const api::resource_desc &desc, D3D11_TEXTURE2D_DESC &internal_desc);
	void convert_resource_desc(const api::resource_desc &desc, D3D11_TEXTURE3D_DESC &internal_desc);
	api::resource_desc convert_resource_desc(const D3D11_BUFFER_DESC &internal_desc);
	api::resource_desc convert_resource_desc(const D3D11_TEXTURE1D_DESC &internal_desc);
	api::resource_desc convert_resource_desc(const D3D11_TEXTURE2D_DESC &internal_desc);
	api::resource_desc convert_resource_desc(const D3D11_TEXTURE3D_DESC &internal_desc);

	void convert_depth_stencil_view_desc(const api::resource_view_desc &desc, D3D11_DEPTH_STENCIL_VIEW_DESC &internal_desc);
	api::resource_view_desc convert_depth_stencil_view_desc(const D3D11_DEPTH_STENCIL_VIEW_DESC &internal_desc);

	void convert_render_target_view_desc(const api::resource_view_desc &desc, D3D11_RENDER_TARGET_VIEW_DESC &internal_desc);
	void convert_render_target_view_desc(const api::resource_view_desc &desc, D3D11_RENDER_TARGET_VIEW_DESC1 &internal_desc);
	api::resource_view_desc convert_render_target_view_desc(const D3D11_RENDER_TARGET_VIEW_DESC &internal_desc);
	api::resource_view_desc convert_render_target_view_desc(const D3D11_RENDER_TARGET_VIEW_DESC1 &internal_desc);

	void convert_shader_resource_view_desc(const api::resource_view_desc &desc, D3D11_SHADER_RESOURCE_VIEW_DESC &internal_desc);
	void convert_shader_resource_view_desc(const api::resource_view_desc &desc, D3D11_SHADER_RESOURCE_VIEW_DESC1 &internal_desc);
	api::resource_view_desc convert_shader_resource_view_desc(const D3D11_SHADER_RESOURCE_VIEW_DESC &internal_desc);
	api::resource_view_desc convert_shader_resource_view_desc(const D3D11_SHADER_RESOURCE_VIEW_DESC1 &internal_desc);

	class device_impl : public api::device
	{
		friend class runtime_d3d11;

	public:
		explicit device_impl(ID3D11Device *device);
		~device_impl();

		bool get_data(const uint8_t guid[16], uint32_t size, void *data) override { return SUCCEEDED(_device->GetPrivateData(*reinterpret_cast<const GUID *>(guid), &size, data)); }
		void set_data(const uint8_t guid[16], uint32_t size, const void *data) override { _device->SetPrivateData(*reinterpret_cast<const GUID *>(guid), size, data); }

		api::render_api get_api() override { return api::render_api::d3d11; }

		bool check_format_support(uint32_t format, api::resource_usage usage) override;

		bool is_resource_valid(api::resource_handle resource) override;
		bool is_resource_view_valid(api::resource_view_handle view) override;

		bool create_resource(const api::resource_desc &desc, api::resource_usage initial_state, api::resource_handle *out_resource) override;
		bool create_resource_view(api::resource_handle resource, const api::resource_view_desc &desc, api::resource_view_handle *out_view) override;

		void destroy_resource(api::resource_handle resource) override;
		void destroy_resource_view(api::resource_view_handle view) override;

		void get_resource_from_view(api::resource_view_handle view, api::resource_handle *out_resource) override;

		api::resource_desc get_resource_desc(api::resource_handle resource) override;

		void wait_idle() override { /* NOP */ }

		void register_resource(ID3D11Resource *resource) { _resources.register_object(resource); }
		void register_resource_view(ID3D11View *resource_view) { _views.register_object(resource_view); }

	private:
		com_ptr<ID3D11Device> _device;
		com_object_list<ID3D11View> _views;
		com_object_list<ID3D11Resource> _resources;
	};

	class device_context_impl : public api::command_queue, public api::command_list
	{
		friend class runtime_d3d11;

	public:
		device_context_impl(device_impl *device, ID3D11DeviceContext *context);
		~device_context_impl();

		bool get_data(const uint8_t guid[16], uint32_t size, void *data) override { return SUCCEEDED(_device_context->GetPrivateData(*reinterpret_cast<const GUID *>(guid), &size, data)); }
		void set_data(const uint8_t guid[16], uint32_t size, const void *data) override { _device_context->SetPrivateData(*reinterpret_cast<const GUID *>(guid), size, data); }

		api::device *get_device() override { return _device_impl; }

		api::command_list *get_immediate_command_list() override { return this; }

		void transition_state(api::resource_handle, api::resource_usage, api::resource_usage) override { /* NOP */ }

		void clear_depth_stencil_view(api::resource_view_handle dsv, uint32_t clear_flags, float depth, uint8_t stencil) override;
		void clear_render_target_view(api::resource_view_handle rtv, const float color[4]) override;

		void copy_resource(api::resource_handle source, api::resource_handle dest) override;

	private:
		device_impl *const _device_impl;
		const com_ptr<ID3D11DeviceContext> _device_context;
	};

	class command_list_impl : public api::command_list
	{
	public:
		command_list_impl(device_impl *device, ID3D11CommandList *cmd_list);
		~command_list_impl();

		bool get_data(const uint8_t guid[16], uint32_t size, void *data) override { return SUCCEEDED(_cmd_list->GetPrivateData(*reinterpret_cast<const GUID *>(guid), &size, data)); }
		void set_data(const uint8_t guid[16], uint32_t size, const void *data) override { _cmd_list->SetPrivateData(*reinterpret_cast<const GUID *>(guid), size, data); }

		api::device *get_device() override { return _device_impl; }

		void transition_state(api::resource_handle, api::resource_usage, api::resource_usage) override { /* NOP */ }

		void clear_depth_stencil_view(api::resource_view_handle dsv, uint32_t clear_flags, float depth, uint8_t stencil) override { assert(false); }
		void clear_render_target_view(api::resource_view_handle rtv, const float color[4]) override { assert(false); }

		void copy_resource(api::resource_handle source, api::resource_handle dest) override { assert(false); }

	private:
		device_impl *const _device_impl;
		const com_ptr<ID3D11CommandList> _cmd_list;
	};
}
